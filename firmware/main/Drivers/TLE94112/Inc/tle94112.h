#ifndef TLE94112_H
#define TLE94112_H

#include "stm32f1xx_hal.h"
#include "stdint.h"

extern SPI_HandleTypeDef hspi1;

struct STM32_Pin {
    GPIO_TypeDef* port;
    uint16_t pin;
};

// Global Enable pin for the TLE94112 drivers
extern STM32_Pin DRIVER_EN;

// Register Adresses
#define HB_ACT_1_CTRL_ADDR      0b10000011
#define HB_ACT_2_CTRL_ADDR      0b11000011
#define HB_ACT_3_CTRL_ADDR      0b10100011
#define HB_MODE_1_CTRL_ADDR     0b11100011
#define HB_MODE_2_CTRL_ADDR     0b10010011
#define HB_MODE_3_CTRL_ADDR     0b11010011
#define PWM_CH_FREQ_CTRL_ADDR   0b10110011
#define PWM1_DC_CTRL_ADDR       0b11110011
#define PWM2_DC_CTRL_ADDR       0b10001011
#define PWM3_DC_CTRL_ADDR       0b11001011
#define FW_OL_CTRL_ADDR         0b10101011
#define FW_CTRL_ADDR            0b11101011
#define CONFIG_CTRL_ADDR        0b01100111

// PWM SETTINGS
#define PWM_CH_FREQ_CTRL_DATA   0b00111111 // PWM 1, 2, 3 at 200Hz
#define PWM1_DC_CTRL_DATA       0b01000000
#define PWM2_DC_CTRL_DATA       0b01000000
#define PWM3_DC_CTRL_DATA       0b01000000
#define FW_OL_CTRL_DATA         0b01001011 // Active freewheeling on LS (HB1,2,4,7)
#define FW_CTRL_DATA            0b00001010 // Active freewheeling on LS (HB10, 12)
#define CLEAR_REG_DATA          0b00000000

enum movement {
    DOWN,
    UP,
    KEEP
};

class MOTOR {
    public:
        movement motor_drive;
        uint8_t current_phase;

        MOTOR();
        void set_direction(movement mov);
};

class TLE94112 {
    private:
        const STM32_Pin *CS_PIN;

        uint16_t reverse_16bit(uint16_t x);

        void spi_transmit_16(uint16_t data);
        void delay_us(uint32_t us);
    public:
        MOTOR M1, M2, M3;

        TLE94112(const STM32_Pin *CS);
        ~TLE94112();

        void spi_transmit_16_cs(uint16_t data);
        void init();
        void start();
        void stop();
        void step_motors(uint32_t num_steps, uint32_t speed_delay_ms);
};

#endif // TLE94112_H