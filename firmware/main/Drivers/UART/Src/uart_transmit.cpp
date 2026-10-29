#include <cstring>

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_uart.h"

#include "uart_transmit.h"

namespace UART {

    // --------- UART_PORT ---------
    static UART_PORT* s_usart1_port = nullptr;
    bool UART_PORT::_setupHardwareInstance() {              // followed setup.c structure for pin and GPIO initialization
        __HAL_RCC_GPIOA_CLK_ENABLE();                       // both pins are on pin A, so this line enables both
        __HAL_RCC_USART1_CLK_ENABLE();                      // turn on uart1

        GPIO_InitTypeDef GPIO_InitStruct = {0};

        GPIO_InitStruct.Pin = _tx_pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;             // pulls for peripheral device
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

        HAL_GPIO_Init(_tx_port, &GPIO_InitStruct);          // configured the data in silicon, we can reuse the same struct

        GPIO_InitStruct.Pin = _rx_pin;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;                 // pullup resistor to be physically implemented on PCB
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

        HAL_GPIO_Init(_rx_port, &GPIO_InitStruct);

        return true;
    }

    bool UART_PORT::_setupTxDma() {
        __HAL_RCC_DMA1_CLK_ENABLE();

        _hdma_tx.Instance = DMA1_Channel4;
        _hdma_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
        _hdma_tx.Init.PeriphInc = DMA_PINC_DISABLE;
        _hdma_tx.Init.MemInc = DMA_MINC_ENABLE;
        _hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        _hdma_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        _hdma_tx.Init.Mode = DMA_NORMAL;
        _hdma_tx.Init.Priority = DMA_PRIORITY_LOW;
        if (HAL_DMA_Init(&_hdma_tx) != HAL_OK) return false;

        __HAL_LINKDMA(&_huart, hdmatx, _hdma_tx);  // HAL_UART_Transmit_DMA looks for the channel in _huart.hdmatx

        HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
        return true;
    }

    UART_PORT::UART_PORT(
        GPIO_TypeDef* tx_port, uint16_t tx_pin,
        GPIO_TypeDef* rx_port, uint16_t rx_pin,
        uint32_t baud
    ) {
        _huart = {0}; // avoid garbage
        _tx_port = tx_port;
        _tx_pin = tx_pin;
        _rx_port = rx_port;
        _rx_pin = rx_pin;
        _baud_rate = baud;
        _rx_byte = 0;
        _rx_head = 0; // circular buffer "pointers"
        _rx_tail = 0;
        _rx_dropped = 0;

        _tx_head = 0;
        _tx_tail = 0;
        _tx_chunk = 0;
        _tx_busy = false;
        _tx_mode = INTERRUPT;
        _tx_dropped = 0;
        _hdma_tx = {0};
    }

    bool UART_PORT::init() {
        _setupHardwareInstance();

        _huart.Instance = USART1; // USART1 is a pointer to the hardware REGISTERS for the peripheral, so it CAN go in the *Instance field of the struct
        _huart.Init.BaudRate        = _baud_rate;
        _huart.Init.WordLength      = UART_WORDLENGTH_8B;   // or UART_WORDLENGTH_9B if we use parity
        _huart.Init.Parity          = UART_PARITY_NONE;     // from what I understand, the 8 bit mode is pretty reliable, we dont need the checksum
        _huart.Init.StopBits        = UART_STOPBITS_1;      // 1 bit is the standard
        _huart.Init.Mode            = UART_MODE_TX_RX;      // duplex
        _huart.Init.HwFlowCtl       = UART_HWCONTROL_NONE;
        _huart.Init.OverSampling    = UART_OVERSAMPLING_16; // mentioned in stm32f1xx_hal_uart.h
                                                            /*!< Specifies whether the Over sampling 8 is enabled or disabled, to achieve higher speed (up to fPCLK/8).
                                                            // This parameter can be a value of @ref UART_Over_Sampling. This feature is only available
                                                            // on STM32F100xx family, so OverSampling parameter should always be set to 16. */
        int rc = HAL_UART_Init(&_huart);
        if (rc != HAL_OK) return false;
        if (!_setupTxDma()) return false;
        s_usart1_port = this;
        HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
        HAL_UART_Receive_IT(&_huart, &_rx_byte, 1);
        return true;
    }

    void UART_PORT::onRxComplete() {
        uint8_t local = (_rx_head + 1) % RX_BUF_SIZE;
        if (local == _rx_tail) {
            _rx_dropped++;
        } else {
            _rx_buf[_rx_head] = _rx_byte;
            _rx_head = local;
        }
        HAL_UART_Receive_IT(&_huart, &_rx_byte, 1);
    }

    bool UART_PORT::read(uint8_t &out) {
        if (_rx_tail == _rx_head) {
            return false;
        }
        out = _rx_buf[_rx_tail];
        _rx_tail = (_rx_tail + 1) % RX_BUF_SIZE;
        return true;
    }

    void UART_PORT::onRxError() {
        HAL_UART_Receive_IT(&_huart, &_rx_byte, 1);
    }

    void UART_PORT::_txStartChunk() {
        uint16_t head = _tx_head;  // we try to reduce the number of volatile opeartions
        uint16_t tail = _tx_tail;

        _tx_chunk = (head > tail) ? (head - tail) : (TX_BUF_SIZE - tail);
        HAL_StatusTypeDef rc = HAL_ERROR;

        if (_tx_mode == INTERRUPT) {
            rc = HAL_UART_Transmit_IT(&_huart, &_tx_buf[tail], _tx_chunk);
        } else if (_tx_mode == DMA) {
            rc = HAL_UART_Transmit_DMA(&_huart, &_tx_buf[tail], _tx_chunk);
        }
        if (rc != HAL_OK) _tx_busy = false;
    }

    void UART_PORT::onTxComplete() {
        _tx_tail = (_tx_tail + _tx_chunk) % TX_BUF_SIZE;
        if (_tx_tail != _tx_head) {
            _txStartChunk();
        } else {
            _tx_busy = false;
        }
    }

    void UART_PORT::onTxError() {
        // A DMA error ends the transfer Callback.
        // An RX error during TX leaves gState BUSY_TX
        if (_tx_busy && _huart.gState == HAL_UART_STATE_READY) {
            onTxComplete(); // if we get here, we lost the chunk, move ahead
        }
    }

    uint16_t UART_PORT::txFree() const {
        return (uint16_t)(_tx_tail - _tx_head - 1) % TX_BUF_SIZE;
    }

    bool UART_PORT::transmitBlocking(const uint8_t* data, uint16_t len) {
        while (_tx_busy) {}
        int timeout = 1000;
        return HAL_OK == HAL_UART_Transmit(&_huart, data, len, timeout);
    }

    bool UART_PORT::enqueue(const uint8_t* data, uint16_t len, TRANSMIT_MODE mode) {
        if (len == 0) return true;
        if (len > txFree()) {
            _tx_dropped += len;
            return false;
        }

        uint16_t head = _tx_head;
        for (uint16_t i = 0; i < len; i++) {
            _tx_buf[head] = data[i];
            head = (head + 1) % TX_BUF_SIZE;
        }
        _tx_head = head;

        // Start the chain if it is stopped. The check and the start must not be split by the
        // TX-complete interrupt, otherwise both sides can think the other one will send.
        HAL_NVIC_DisableIRQ(USART1_IRQn);
        _tx_mode = mode;
        if (!_tx_busy) {
            _tx_busy = true;
            _txStartChunk();
        }
        HAL_NVIC_EnableIRQ(USART1_IRQn);
        return true;
    }

    bool UART_PORT::setBaudRate(uint32_t baud) {
        while (_tx_busy) {
            ;
        }

        HAL_UART_AbortReceive(&_huart); // turn RXNEIE off; otherwise HAL_UART_Init sets RxState READY with RXNEIE still on,

        _baud_rate = baud;
        _huart.Init.BaudRate = baud;
        if (HAL_UART_Init(&_huart) != HAL_OK) return false;

        HAL_UART_Receive_IT(&_huart, &_rx_byte, 1);
        return true;
    }

    uint32_t UART_PORT::getBaudRate() const {
        return _baud_rate;
    }

    UART_HandleTypeDef* UART_PORT::getHandle() {
        return &_huart;
    }

    DMA_HandleTypeDef* UART_PORT::getTxDmaHandle() {
        return &_hdma_tx;
    }

    // --------- UART_DEVICE ---------

    bool UART_DEVICE::print(const char* msg, TRANSMIT_MODE mode) {
        uint16_t len = strlen(msg);

        if (mode == BLOCKING) {
            return _port.transmitBlocking((const uint8_t*)msg, len);
        } else if (mode == INTERRUPT) {
            return _port.enqueue((const uint8_t*)msg, len, INTERRUPT);
        } else if (mode == DMA) {
            return _port.enqueue((const uint8_t*)msg, len, DMA);
        } else {
            return false;
        }
    }

    bool UART_DEVICE::println(const char* msg, TRANSMIT_MODE mode) {
        if (mode != BLOCKING && _port.txFree() < strlen(msg) + 2) {
            return false;
        }

        return print(msg, mode) && print("\r\n", mode);
    }

    bool UART_DEVICE::printf(const char* format, ...) {
        char buffer[256];

        va_list args;
        va_start(args, format);

        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        return print(buffer, DMA);
    }
}

extern "C" void USART1_IRQHandler(void) {
    if (UART::s_usart1_port == nullptr) return;
    HAL_UART_IRQHandler(UART::s_usart1_port->getHandle());
}

extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (UART::s_usart1_port == nullptr) return;
    if (huart != UART::s_usart1_port->getHandle()) return;
    UART::s_usart1_port->onRxComplete();
}

extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (UART::s_usart1_port == nullptr) return;
    if (huart != UART::s_usart1_port->getHandle()) return;
    UART::s_usart1_port->onTxComplete();
}

extern "C" void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
    if (UART::s_usart1_port == nullptr) return;
    if (huart != UART::s_usart1_port->getHandle()) return;
    UART::s_usart1_port->onRxError();
    UART::s_usart1_port->onTxError();
}

extern "C" void DMA1_Channel4_IRQHandler(void) {
    if (UART::s_usart1_port == nullptr) return;
    HAL_DMA_IRQHandler(UART::s_usart1_port->getTxDmaHandle());
}