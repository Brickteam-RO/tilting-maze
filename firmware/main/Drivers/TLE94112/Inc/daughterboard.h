#ifndef DAUGHTERBOARD_H
#define DAUGHTERBOARD_H

#include "stm32f1xx_hal.h"
#include "tle94112.h"

extern SPI_HandleTypeDef hspi1;

// DEMUX-SELECTION & SPI Pins
#define SEL_0_PORT GPIOB
#define SEL_0_PIN GPIO_PIN_5
#define SEL_1_PORT GPIOB
#define SEL_1_PIN GPIO_PIN_6
#define SEL_2_PORT GPIOB
#define SEL_2_PIN GPIO_PIN_7
#define SPI_CS_PORT GPIOA
#define SPI_CS_PIN GPIO_PIN_4

class DaughterBoard {
    private:
        uint8_t board_id;
        void delay_us(uint32_t us);

    public:
        TLE94112 drivers[7];
        DaughterBoard(uint8_t id);
        ~DaughterBoard();

        void select_board();
        void transmit_daisy_chain(uint8_t reg_addr);
        void transmit_daisy_chain(uint8_t reg_addr, uint8_t fixed_data);

        void init();
        void start();
        void stop();

        void step_motors(uint32_t num_steps, uint32_t speed_delay_ms);
};

#endif // DAUGHTERBOARD_H