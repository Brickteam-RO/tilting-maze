#include "tle94112.h"

MOTOR::MOTOR() : motor_drive(KEEP), current_phase(0) {}

void MOTOR::set_direction(movement mov) {
    motor_drive = mov;
}

TLE94112::TLE94112() {
    reset_state();
}

void TLE94112::clear_step_data() {
    act_1_data = 0;
    act_2_data = 0;
    act_3_data = 0;
    mode_1_data = 0;
    mode_2_data = 0;
    mode_3_data = 0;
}

void TLE94112::reset_state() {
    clear_step_data();
    M1.set_direction(KEEP);
    M2.set_direction(KEEP);
    M3.set_direction(KEEP);
}

void TLE94112::calculate_step_data() {
    clear_step_data();
    if(M1.motor_drive != KEEP) {
        mode_1_data |= 0b00000101;
        mode_2_data |= 0b01000001;

        if(M1.motor_drive == UP) {
            M1.current_phase = (M1.current_phase + 1) % 4;
        }
        else { // DOWN
            M1.current_phase = (M1.current_phase == 0) ? 3 : (M1.current_phase - 1);
        }

        switch(M1.current_phase) {
            case 0: // Step 1: Coil A (+), Coil B (+)
                act_1_data |= 0b00000110;
                act_2_data |= 0b01000010;
                break;
            case 1: // Step 2: Coil A (-), Coil B (+)
                act_1_data |= 0b00001001;
                act_2_data |= 0b01000010;
                break;
            case 2: // Step 3: Coil A (-), Coil B (-)
                act_1_data |= 0b00001001;
                act_2_data |= 0b10000001;
                break;
            case 3: // Step 4: Coil A (+), Coil B (-)
                act_1_data |= 0b00000110;
                act_2_data |= 0b10000001;
                break;
        }
    }

    if(M2.motor_drive != KEEP) {
        mode_1_data |= 0b10000000;
        mode_2_data |= 0b00101000;
        mode_3_data |= 0b00000010;

        if(M2.motor_drive == UP) {
            M2.current_phase = (M2.current_phase + 1) % 4;
        }
        else { // DOWN
            M2.current_phase = (M2.current_phase == 0) ? 3 : (M2.current_phase - 1);
        }

        switch(M2.current_phase) {
            case 0: // Step 1: Coil A (+), Coil B (+)
                act_1_data |= 0b01000000;
                act_2_data |= 0b00101000;
                act_3_data |= 0b00000001;
                break;
            case 1: // Step 2: Coil A (-), Coil B (+)
                act_1_data |= 0b01000000;
                act_2_data |= 0b00011000;
                act_3_data |= 0b00000010;
                break;
            case 2: // Step 3: Coil A (-), Coil B (-)
                act_1_data |= 0b10000000;
                act_2_data |= 0b00010100;
                act_3_data |= 0b00000010;
                break;
            case 3: // Step 4: Coil A (+), Coil B (-)
                act_1_data |= 0b10000000;
                act_2_data |= 0b00100100;
                act_3_data |= 0b00000001;
                break;
        }
    }

    if(M3.motor_drive != KEEP) {
        mode_1_data |= 0b00110000;
        mode_3_data |= 0b11111100;

        if(M3.motor_drive == UP) {
            M3.current_phase = (M3.current_phase + 1) % 4;
        }
        else { // DOWN
            M3.current_phase = (M3.current_phase == 0) ? 3 : (M3.current_phase - 1);
        }

        switch(M3.current_phase) {
            case 0: // Step 1: Coil A (+), Coil B (+)
                act_3_data |= 0b10011000;
                act_1_data |= 0b00010000;
                break;
            case 1: // Step 2: Coil A (-), Coil B (+)
                act_3_data |= 0b01101000;
                act_1_data |= 0b00010000;
                break;
            case 2: // Step 3: Coil A (-), Coil B (-)
                act_3_data |= 0b01100100;
                act_1_data |= 0b00100000;
                break;
            case 3: // Step 4: Coil A (+), Coil B (-)
                act_3_data |= 0b10010100;
                act_1_data |= 0b00100000;
                break;
        }
    }
}

uint8_t TLE94112::get_register_data(uint8_t reg_addr) const {
    switch(reg_addr) {
        case HB_ACT_1_CTRL_ADDR: return act_1_data;
        case HB_ACT_2_CTRL_ADDR: return act_2_data;
        case HB_ACT_3_CTRL_ADDR: return act_3_data;
        case HB_MODE_1_CTRL_ADDR: return mode_1_data;
        case HB_MODE_2_CTRL_ADDR: return mode_2_data;
        case HB_MODE_3_CTRL_ADDR: return mode_3_data;
        default: return 0;
    }
}