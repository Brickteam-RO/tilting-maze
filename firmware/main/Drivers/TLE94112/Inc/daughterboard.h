#ifndef DAUGHTERBOARD_H
#define DAUGHTERBOARD_H

#include "stm32f1xx_hal.h"
#include "tle94112.h"

extern "C" SPI_HandleTypeDef hspi1;
extern "C" TIM_HandleTypeDef htim1;

// DEMUX-SELECTION & SPI Pins
#define SEL_0_PORT GPIOB
#define SEL_0_PIN GPIO_PIN_5
#define SEL_1_PORT GPIOB
#define SEL_1_PIN GPIO_PIN_6
#define SEL_2_PORT GPIOB
#define SEL_2_PIN GPIO_PIN_7
#define SPI_CS_PORT GPIOA
#define SPI_CS_PIN GPIO_PIN_4

// Mask for LABT(Last Address Bit Token)
#define LABT_MASK 0b00000010

class DaughterBoard {
    private:
        uint8_t board_id;
        void delay_us(uint32_t us);

    public:
        
        TLE94112 drivers[7];

        /**
         * @brief Sets up the board with its ID
         * @param id represents the board ID(0-4)
         */
        DaughterBoard(uint8_t id);
        ~DaughterBoard();

        /**
         * @brief Selects the board on the DEMUX so it can receive SPI data.
         */
        void select_board();

        /**
         * @brief Dynamic SPI function. Asks all 7 chips what bits they need 
         * and send them over daisy chain.
         * @param reg_addr represents the register we want to update.
         */
        void transmit_daisy_chain(uint8_t reg_addr);

        /**
         * @brief Fixed SPI function. Sends the same byte to all 7 chips.
         * @param reg_addr represents the register we want to update.
         * @param fixed_data represents the byte we want to send to all chips.
         */
        void transmit_daisy_chain(uint8_t reg_addr, uint8_t fixed_data);

        /**
         * @brief Clears old data and turns on Freewheeling.
         */
        void init();

        /**
         * @brief Safely turns on the PWM signal.
         */
        void start();

        /**
         * @brief Turns off the PWM signal and stops all motors.
         */
        void stop();

        /**
         * @brief Calculates and executes one motor step for this board.
         */
        void execute_single_step();
};

#endif // DAUGHTERBOARD_H