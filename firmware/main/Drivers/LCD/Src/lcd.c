#include "lcd.h"
#include "stm32f1xx_hal.h"

#include <stdint.h>
#include <stdbool.h>

static I2C_HandleTypeDef* i2c_handle;
static uint8_t i2c_addr;
static uint8_t backlight_state = 0x08; /* 0x08 - ON, 0x00 - OFF */

void lcd_setup(I2C_HandleTypeDef* handle, uint8_t i2c_address, uint8_t config) {
    if (config & LCD_BACKLIGHT_ON) {
        // TODO: handle backlight setting in setup
    }

    if (config & LCD_CURSOR_SHOW) {
        // TODO: handle cursor show setting in setup
    }

    if (config & LCD_CURSOR_BLINK) {
        // TODO: handle cursor blinking setting in setup
    }

    if (config & LCD_DIR_LEFT) {
        // TODO: handle right-to-left setting in setup
    }

    if (config & LCD_SHIFT_MODE) {
        // TODO: handle shift mode setting in setup
    }
}

void lcd_clear() {
    lcd_transmit(0x01, LCD_CMD);
    HAL_Delay(2);   // the display requires 2 ms to clear its internal RAM
}

void lcd_backlight(bool state) {
    backlight_state = state ? 0x08 : 0x00;
    lcd_transmit(0, LCD_CMD);
}

void lcd_set_cursor(uint8_t row, uint8_t column) {
    
}

void lcd_transmit(uint8_t data, LCD_MODE mode) {
    uint8_t nibble_h = data & 0xF0;
    uint8_t nibble_l = (data << 4) & 0xF0;
    uint8_t data_array[4];

    uint8_t control_bits = backlight_state | mode;

    data_array[0] = nibble_h | control_bits | 0x04;     // EN high
    data_array[1] = nibble_h | control_bits;            // EN low

    data_array[2] = nibble_l | control_bits | 0x04;     // EN high
    data_array[3] = nibble_l | control_bits;            // EN low

    HAL_I2C_Master_Transmit(i2c_handle, i2c_addr, data_array, 4, 100);
}

void lcd_print(const char* format, ...) {

}