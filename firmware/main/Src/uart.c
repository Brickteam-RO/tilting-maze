#include "uart.h"

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_dma.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>

extern UART_HandleTypeDef huart1;

static uart_mode_t uart_mode = UART_MODE_DMA;
static DMA_HandleTypeDef hdma_usart1_tx;
static bool dma_initialized = false;

static uint8_t tx_buffer[256];

static bool uart_dma_init(void)
{
    if (dma_initialized) {
        return true;
    }

    __HAL_RCC_DMA1_CLK_ENABLE();

    hdma_usart1_tx.Instance = DMA1_Channel4;
    hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_tx.Init.Mode = DMA_NORMAL;
    hdma_usart1_tx.Init.Priority = DMA_PRIORITY_LOW;

    if (HAL_DMA_Init(&hdma_usart1_tx) != HAL_OK) {
        return false;
    }

    __HAL_LINKDMA(&huart1, hdmatx, hdma_usart1_tx);

    HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);

    dma_initialized = true;
    return true;
}

void set_uart_mode(uart_mode_t mode)
{
    if (mode == UART_MODE_DMA && !uart_dma_init()) {
        return;
    }

    uart_mode = mode;
}

bool uart_printf(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    int length = vsnprintf(
        (char *)tx_buffer,
        sizeof(tx_buffer),
        fmt,
        args
    );
    va_end(args);

    if (length <= 0 || length >= (int)sizeof(tx_buffer)) {
        return false;
    }

    if (uart_mode == UART_MODE_BLOCKING) {
        return HAL_UART_Transmit(
            &huart1,
            tx_buffer,
            (uint16_t)length,
            HAL_MAX_DELAY
        ) == HAL_OK;
    }

    if (uart_mode == UART_MODE_DMA) {
        if (!dma_initialized && !uart_dma_init()) {
            return false;
        }

        while (HAL_UART_GetState(&huart1) != HAL_UART_STATE_READY) {
        }

        return HAL_UART_Transmit_DMA(
            &huart1,
            tx_buffer,
            (uint16_t)length
        ) == HAL_OK;
    }

    if (uart_mode == UART_MODE_INTERRUPT) {
        return HAL_UART_Transmit_IT(
            &huart1,
            tx_buffer,
            (uint16_t)length
        ) == HAL_OK;
    }

    return false;
}

void DMA1_Channel4_IRQHandler(void)
{
    if (huart1.hdmatx != NULL) {
        HAL_DMA_IRQHandler(huart1.hdmatx);
    }
}