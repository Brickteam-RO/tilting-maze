#pragma once

#include <cstdint>
#include <cstdarg>
#include <cstdio>

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_uart.h"

namespace UART {
    enum TRANSMIT_MODE {
        /** @brief Used for priority transmission, blocks CPU until end of message */
        BLOCKING,
        /** @brief Used for non-priority transmission, does NOT block CPU during transmission. One interrupt per byte. */
        INTERRUPT,
        /** @brief For Direct Memory Access */
        DMA
    };

    /** @brief UART port definition class */
    class UART_PORT {
    private:
        UART_HandleTypeDef _huart;  // in stm32f1xx_hal_uart.h, same with its Init struct

        GPIO_TypeDef* _tx_port;
        uint16_t _tx_pin;

        GPIO_TypeDef* _rx_port;
        uint16_t _rx_pin;

        uint32_t _baud_rate;
        static const uint8_t RX_BUF_SIZE = 64;
        uint8_t _rx_byte;
        uint8_t _rx_buf[RX_BUF_SIZE];
        volatile uint8_t _rx_head;
        volatile uint8_t _rx_tail;
        volatile uint8_t _rx_dropped;

        static const uint16_t TX_BUF_SIZE = 256;
        uint8_t _tx_buf[TX_BUF_SIZE];
        volatile uint16_t _tx_head;
        volatile uint16_t _tx_tail;
        volatile uint16_t _tx_chunk;
        volatile bool _tx_busy;
        volatile TRANSMIT_MODE _tx_mode; // INTERRUPT or DMA
        volatile uint16_t _tx_dropped; // bytes thrown away because the buffer was full and we dont block

        DMA_HandleTypeDef _hdma_tx;

        bool _setupHardwareInstance();

        /** @brief configures DMA1 Channel 4 for USART1_TX and links it to _huart */
        bool _setupTxDma();

        /** @brief starts sending the longest contiguous chunk from _tx_tail. We assum !buffer.empty(). */
        void _txStartChunk();

    public:

        UART_PORT(GPIO_TypeDef* tx_port, uint16_t tx_pin,
                  GPIO_TypeDef* rx_port, uint16_t rx_pin,
                  uint32_t baud = 115200);

        /**
         * @brief Configures the clocks, the pins and the USART1. Uses 8N1 settings.
         *
         * @return true if HAL_UART_Init() returned HAL_OK, false otherwise
         */
        bool init();

        bool setBaudRate(uint32_t baud);
        uint32_t getBaudRate() const;
        UART_HandleTypeDef* getHandle();
        DMA_HandleTypeDef* getTxDmaHandle();

        void onRxComplete();
        void onRxError();
        void onTxComplete();
        void onTxError();
        bool read(uint8_t &out);

        /** @brief free space in the TX buffer. */
        uint16_t txFree() const;

        /** @brief waits for the queued bytes to go out, then sends these with HAL_UART_Transmit. */
        bool transmitBlocking(const uint8_t* data, uint16_t len);

        /** @brief INTERRUPT / DMA : copies the bytes into the TX buffer, then returns */
        bool enqueue(const uint8_t* data, uint16_t len, TRANSMIT_MODE mode);
    };

    /** @brief UART device class */
    class UART_DEVICE {
    private:
        UART_PORT _port;

    public:


        UART_DEVICE (
            GPIO_TypeDef* tx_port, uint16_t tx_pin, 
            GPIO_TypeDef* rx_port, uint16_t rx_pin, 
            uint32_t baud = 115200
        ) : _port(tx_port, tx_pin, rx_port, rx_pin, baud) {};

        /**
         * @brief Initializes the UART device
         * 
         * @return true if port succeeded, false otherwise
         */
        bool init() { return _port.init(); };

        /**
         * @brief Takes the oldest received byte out of the RX buffer. Does not block.
         *
         * @param out where the byte is written; left unchanged if the buffer is empty
         *
         * @return true if a byte was read, false if the buffer was empty
         */
        bool read(uint8_t &out) { return _port.read(out); };

        /**
         * @brief Prints a message to the UART device.
         *
         * @param msg pointer to a string, the message to be printed
         * @param mode the mode of transmission: BLOCKING or INTERRUPT or DMA 
         *
         * @return INTERRUPT / DMA : true if the message was queued, false if it was dropped
         */
        bool print(const char* msg, TRANSMIT_MODE mode = BLOCKING);

        /** @brief Prints a message to the UART device, followed by a newline character. */
        bool println(const char* msg, TRANSMIT_MODE mode = BLOCKING);
        
        /**
         * @brief Prints a formatted message using DMA mode
         * 
         * @param format Format string(printf style)
         * @param ... Variable arguments for formatting
         * @return true if successful, false otherwise.
         */
        bool printf(const char* format, ...);
    };
}