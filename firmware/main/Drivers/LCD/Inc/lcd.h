#pragma once

#include "stm32f1xx_hal.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

typedef enum {
    LCD_CMD,
    LCD_DATA
} LCD_MODE;

/**
 * @brief Enum for ```lcd_setup()``` config
 */
typedef enum LCD_CONFIG_FLAG {
    LCD_BACKLIGHT_ON    = 1,      /*!< enables display backlight */
    LCD_CURSOR_SHOW     = 1 << 1, /*!< enables cursor (underline) */
    LCD_CURSOR_BLINK    = 1 << 2, /*!< enables cursor blinking */
    LCD_DIR_LEFT        = 1 << 3, /*!< sets text direction to RIGHT-TO-LEFT (omit if LEFT-TO-RIGHT text) */
    LCD_SHIFT_MODE      = 1 << 4  /*!< makes display shift with text */
} LCD_CONFIG_FLAG;

/* Sets LCD display configuration using config bitmask */
void lcd_setup(I2C_HandleTypeDef* handle, uint8_t i2c_address, uint8_t config);

/* Clears LCD display */
void lcd_clear();

/* Sets backlight state */
void lcd_backlight(bool state);

/* Sets cursor position (which indicates where to write, not just visual) */
void lcd_set_cursor(uint8_t row, uint8_t column);

/* Transmit data to LCD in either command mode or data mode */
void lcd_transmit(uint8_t data, LCD_MODE mode);

/* Print formatted string to LCD */
void lcd_print(const char* format, ...);