#pragma once

#include <stdint.h>
#include "l298n.h"

/**
 * @brief Direction in which the maze should tilt.
 */
enum class TiltDirection : uint8_t {
    LEFT,
    RIGHT,
    UP,
    DOWN
};

/**
 * @brief High-level controller for four actuators connected to two L298N drivers.
 */
class TiltController {
public:
    /**
     * @brief Constructs a tilt controller.
     *
     * @param driver1 Reference to the first L298N driver.
     * @param driver2 Reference to the second L298N driver.
     */
    TiltController(L298N& driver1, L298N& driver2);

    /**
     * @brief Initializes both L298N drivers and stops all actuators.
     */
    void init();

    /**
     * @brief Tilts the maze in the specified direction.
     *
     * @param direction Desired tilt direction.
     */
    void tilt(TiltDirection direction);

    /**
     * @brief Tilts the maze toward the left.
     *
     * The left-side motors are lowered and the right-side motors are raised.
     */
    void tilt_left();

    /**
     * @brief Tilts the maze toward the right.
     *
     * The right-side motors are lowered and the left-side motors are raised.
     */
    void tilt_right();

    /**
     * @brief Tilts the maze toward the top.
     *
     * The bottom-side motors are raised and the top-side motors are lowered.
     */
    void tilt_up();

    /**
     * @brief Tilts the maze toward the bottom.
     *
     * The top-side motors are raised and the bottom-side motors are lowered.
     */
    void tilt_down();

    /**
     * @brief Raises all four actuators.
     */
    void raise_all();

    /**
     * @brief Lowers all four actuators.
     */
    void lower_all();

    /**
     * @brief Stops all four actuators.
     */
    void stop_all();

private:
    L298N& _driver1;
    L298N& _driver2;

    void raise_left_side();
    void lower_left_side();

    void raise_right_side();
    void lower_right_side();

    void raise_top_side();
    void lower_top_side();

    void raise_bottom_side();
    void lower_bottom_side();
};