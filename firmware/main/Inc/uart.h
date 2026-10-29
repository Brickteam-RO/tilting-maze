#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enum for UART operation modes.
 */
typedef enum {
    UART_MODE_BLOCKING,
    UART_MODE_DMA,
    UART_MODE_INTERRUPT
} uart_mode_t;

/**
 * @brief Initializes the UART peripheral with the specified mode.
 * 
 * @param mode The desired UART mode (blocking, DMA, or interrupt).
 */
void set_uart_mode(uart_mode_t mode);

/**
 * @brief Transmits data over UART.
 * 
 * @param data Pointer to the data buffer to transmit.
 * @param length Length of the data to transmit.
 * @return true if the transmission was successful, false otherwise.
 */
bool uart_printf(const char* fmt, ...);

#ifdef __cplusplus
}
#endif