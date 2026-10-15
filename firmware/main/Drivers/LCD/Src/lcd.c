#include "lcd.h"
#include "stm32f1xx_hal.h"

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

static I2C_HandleTypeDef* i2c_handle;
static uint8_t i2c_addr;
static uint8_t backlight_state = 0x08; /* 0x08 - ON, 0x00 - OFF */

void lcd_setup(I2C_HandleTypeDef* handle, uint8_t i2c_address, uint8_t config) {
    i2c_addr = i2c_address << 1;
    i2c_handle = handle;
    uint8_t data = 0;
    backlight_state = 0x00;
    if (config & LCD_BACKLIGHT_ON) {
        backlight_state = 0x08;
    }

    HAL_Delay(200);
    if (HAL_I2C_IsDeviceReady(i2c_handle, i2c_addr, 3, 100) != HAL_OK) {
        // I2C communication failed - wrong address or bad bus connection
        while (1); 
    }
    
    HAL_Delay(50); 
    
    uint8_t init_cmds[] = {0x30, 0x30, 0x30, 0x20};
    uint8_t data_arr[2];
    
    for (int i = 0; i < 4; i++) {   // power up sequence
        data_arr[0] = init_cmds[i] | backlight_state | 0x04; // EN High
        data_arr[1] = init_cmds[i] | backlight_state;        // EN Low
        HAL_I2C_Master_Transmit(i2c_handle, i2c_addr, data_arr, 2, 100);
        
        if (i == 0) HAL_Delay(5);
        else HAL_Delay(1);
    }

    data = 1 << 5;  // function set command
    data |= 0 << 4; // 4 bit transmission
    data |= 1 << 3; // 2 line configuration
    data |= 0 << 2; // 5x8 dots font
    lcd_transmit(data, LCD_CMD);
    
    lcd_clear();


    data = 1 << 3 | 1 << 2; // display control (enable display bit(2) set)
    if (config & LCD_CURSOR_SHOW) {
        data |= 1 << 1; // enable cursor bit
    }

    if (config & LCD_CURSOR_BLINK) {
        data |= 1 << 0; // enable cursor blinking bit
    }

    // transmit display control command
    lcd_transmit(data, LCD_CMD);
    HAL_Delay(1);


    data = (1 << 2) | (1 << 1);
    if (config & LCD_DIR_LEFT) {
        data &= ~(1 << 1);      // Decrement (right-to-left)
    }
    if (config & LCD_SHIFT_MODE) {
        data |= (1 << 0);
    }

    // transmit entry mode command
    lcd_transmit(data, LCD_CMD);
    HAL_Delay(1);


    // move cursor to home
    lcd_transmit(1 << 1, LCD_CMD);
    HAL_Delay(2);   // display requires 2 ms to move cursor back to start position
}

void lcd_clear() {
    lcd_transmit(0x01, LCD_CMD);
    HAL_Delay(2);   // the display requires 2 ms to clear its internal RAM
}

void lcd_backlight(bool state) {
    backlight_state = state ? 0x08 : 0x00;
    uint8_t data = backlight_state;
    HAL_I2C_Master_Transmit(i2c_handle, i2c_addr, &data, 1, 100);
}

void lcd_set_cursor(uint8_t row, uint8_t column) {
    uint8_t row_offsets[] = {0x00, 0x40};
    if (row > 1) row = 1;
    if (column > 15) column = 15;

    lcd_transmit(0x80 | (row_offsets[row] + column), LCD_CMD);
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
    char buf[64];

    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);

    for (size_t i = 0; buf[i] != '\0'; i++) {
        lcd_transmit((uint8_t)buf[i], LCD_DATA);
    }
}