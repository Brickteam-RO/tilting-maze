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

        MOTOR();
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

        TLE94112();
        void clear_step_data();
        void calculate_step_data();
        void reset_state();
        uint8_t get_register_data(uint8_t reg_addr) const;
};

#endif // TLE94112_H