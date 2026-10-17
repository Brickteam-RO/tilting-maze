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
        
        /**
         * @brief Sets up all 5 boards and marks all cubes as DOWN.
         */
        MAZE();

        /**
         * @brief Initializes the hardware on all 5 boards.
         */
        void init();
        
        /**
         * @brief Adds a "moveUP" command to the queue for the cube at (i,j).
         */
        void prepare_raise(uint8_t i, uint8_t j);

        /**
         * @brief Adds a "move DOWN" command to the queue for the cube at (i,j).
         */
        void prepare_lower(uint8_t i, uint8_t j);

        /**
         * @brief Executes all the prepared movements at the same time.
         * @param num_steps represents how many steps the motors should take.
         * @param speed_delay_ms represents the pause between steps.
         */
        void execute_movements(uint32_t num_steps, uint32_t speed_delay_ms);
        
        /**
         * @brief Checks if the cube at (i,j) is UP or DOWN.
         */
        CubeState get_status(uint8_t i, uint8_t j);
};

#endif // MAZE_H