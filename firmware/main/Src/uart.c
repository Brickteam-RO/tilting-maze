#include "uart.h"

#include "stm32f1xx_hal.h"

#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

extern UART_HandleTypeDef huart1;
static uart_mode_t uart_mode = UART_MODE_DMA; // Default mode

bool uart_printf(const char* fmt, ...) {
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    // transmit the formatted string over UART using the mode set in uart_mode_t
    HAL_StatusTypeDef status;
    if (uart_mode == UART_MODE_BLOCKING) {
        status = HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
    } else if (uart_mode == UART_MODE_DMA) {
        status = HAL_UART_Transmit_DMA(&huart1, (uint8_t*)buffer, strlen(buffer));
    } else if (uart_mode == UART_MODE_INTERRUPT) {
        status = HAL_UART_Transmit_IT(&huart1, (uint8_t*)buffer, strlen(buffer));
    } else {
        return false; // Invalid mode
    }

    return (status == HAL_OK);
}

void set_uart_mode(uart_mode_t mode) {
    // Implementation for setting the UART mode
}