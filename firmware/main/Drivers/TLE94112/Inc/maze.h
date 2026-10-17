#ifndef MAZE_H
#define MAZE_H

#include "daughterboard.h"

enum CubeState {
    CUBE_DOWN = 0,
    CUBE_UP = 1
};

class MAZE{
    private:
        DaughterBoard boards[5];
        CubeState grid_state[10][10];
        struct MotorLocation {
            uint8_t board_index;
            uint8_t driver_index;
            uint8_t motor_id;
        };

        MotorLocation calculate_hardware_location(uint8_t i, uint8_t j);

    public:
        
        MAZE();
        void init();
        
        void prepare_raise(uint8_t i, uint8_t j);
        void prepare_lower(uint8_t i, uint8_t j);
        void execute_movements(uint32_t num_steps, uint32_t speed_delay_ms);
        
        CubeState get_status(uint8_t i, uint8_t j);
};
#endif // MAZE_H