#ifndef TLE94112_H
#define TLE94112_H

#include "stdint.h"

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

        /**
         * @brief Starts the motor in KEEP state.
         */
        MOTOR();
        /**
         * @brief Updates the motor direction.
         * @param mov represents the new direction(UP, DOWN, KEEP).
         */
        void set_direction(movement mov);
};

class TLE94112 {
    private:
        uint8_t act_1_data;
        uint8_t act_2_data;
        uint8_t act_3_data;
        uint8_t mode_1_data;
        uint8_t mode_2_data;
        uint8_t mode_3_data;

    public:
        MOTOR M1, M2, M3;

        /**
         * @brief Sets up the TLE94112 and makes sure all 3 motors are off initially.
         */
        TLE94112();

        /**
         * @brief Fills the register variables with 0 to start.
         */
        void clear_step_data();

        /**
         * @brief Checks what M1, M2 and M3 want to do 
         * and calculates the exact bits needed for SPI transmission.
         */
        void calculate_step_data();

        /**
         * @brief Forces all 3 motors to stop and resets their steps.
         */
        void reset_state();

        /**
         * @brief Gives the bits to the DaughterBoard to transmit over SPI.
         * @param reg_addr represents the register that the board is asking for.
         */
        uint8_t get_register_data(uint8_t reg_addr) const;
};

#endif // TLE94112_H