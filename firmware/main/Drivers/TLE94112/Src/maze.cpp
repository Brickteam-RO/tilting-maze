#include "maze.h"

MAZE::MAZE() : boards{DaughterBoard(0), DaughterBoard(1), DaughterBoard(2), DaughterBoard(3), DaughterBoard(4)} {
    for(int i = 0; i < 10; i++) {
        for(int j = 0; j < 10; j++) {
            grid_state[i][j] = CUBE_DOWN;
        }
    }
}

void MAZE::init() {
    for(int i = 0; i < 5; i++) {
        boards[i].init();
    }
}

MAZE::MotorLocation MAZE::calculate_hardware_location(uint8_t i, uint8_t j) {
    MotorLocation location;

    uint8_t linear_index = (i * 10) + j;
    
    location.board_index = linear_index / 20;
    uint8_t motor_on_board = linear_index % 20;
    location.driver_index = motor_on_board / 3;
    location.motor_id = motor_on_board % 3;

    return location;
}

void MAZE::prepare_raise(uint8_t i, uint8_t j) {
    if(i > 9 || j > 9) return;

    if(grid_state[i][j] == CUBE_UP) return;

    MotorLocation location = calculate_hardware_location(i, j);
    TLE94112& target_driver = boards[location.board_index].drivers[location.driver_index];

    if(location.motor_id == 0) {
        target_driver.M1.set_direction(UP);
    }
    else if(location.motor_id == 1) {
        target_driver.M2.set_direction(UP);
    }
    else if(location.motor_id == 2) {
        target_driver.M3.set_direction(UP);
    }

    grid_state[i][j] = CUBE_UP;
}

void MAZE::prepare_lower(uint8_t i, uint8_t j) {
    if(i > 9 || j > 9) return;

    if(grid_state[i][j] == CUBE_DOWN) return;

    MotorLocation location = calculate_hardware_location(i, j);
    TLE94112& target_driver = boards[location.board_index].drivers[location.driver_index];

    if(location.motor_id == 0) {
        target_driver.M1.set_direction(DOWN);
    }
    else if(location.motor_id == 1) {
        target_driver.M2.set_direction(DOWN);
    }
    else if(location.motor_id == 2) {
        target_driver.M3.set_direction(DOWN);
    }

    grid_state[i][j] = CUBE_DOWN;
}

void MAZE::execute_movements(uint32_t num_steps, uint32_t speed_delay_ms) {
    for(int i = 0; i < 5; i++) {
        boards[i].start();
    }
    
    for(uint32_t step = 0; step < num_steps; step++) {
        for(int i = 0; i < 5; i++) {
            boards[i].execute_single_step();
        }
        HAL_Delay(speed_delay_ms);
    }

    for(int i = 0; i < 5; i++) {
        boards[i].stop();
    }
}

CubeState MAZE::get_status(uint8_t i, uint8_t j) {
    if(i > 9 || j > 9) return CUBE_DOWN;

    return grid_state[i][j];
}