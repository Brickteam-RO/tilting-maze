#pragma once

#include <stdint.h>
#include "stm32f1xx_hal.h"

/**
 * @brief Direction or operating state of an actuator connected to an L298N.
 */
enum l298n_dir : uint8_t {
    DOWN = 0,
    UP   = 1,
    STOP = 2
};

/**
 * @brief Identifies one of the two motor channels on an L298N.
 */
enum l298n_motor : uint8_t {
    MOTOR_A = 0,
    MOTOR_B = 1
};

/**
 * @brief GPIO configuration for one L298N motor channel.
 *
 * Each motor channel uses two L298N input signals:
 *
 *     IN1/IN2 for motor A
 *     IN3/IN4 for motor B
 */
struct L298NMotorPins {
    GPIO_TypeDef* input_a_port;
    uint16_t      input_a_pin;

    GPIO_TypeDef* input_b_port;
    uint16_t      input_b_pin;
};

/**
 * @brief Driver for one dual-channel L298N motor driver.
 */
class L298N {
public:
    /**
     * @brief Constructs an L298N driver object.
     *
     * @param motor_a_pins GPIO configuration for motor channel A.
     * @param motor_b_pins GPIO configuration for motor channel B.
     */
    L298N(
        const L298NMotorPins& motor_a_pins,
        const L298NMotorPins& motor_b_pins
    );

    /**
     * @brief Initializes the four GPIO pins as push-pull outputs.
     *
     * All motor outputs are placed in the STOP state.
     */
    void init();

    /**
     * @brief Commands one motor channel.
     *
     * @param motorid Motor channel, either MOTOR_A or MOTOR_B.
     * @param direction Desired direction: UP, DOWN, or STOP.
     */
    void set_motor(uint8_t motorid, l298n_dir direction);

    /**
     * @brief Stops one motor channel.
     *
     * @param motorid Motor channel, either MOTOR_A or MOTOR_B.
     */
    void stop_motor(uint8_t motorid);

    /**
     * @brief Stops both motor channels connected to this driver.
     */
    void stop_all();

private:
    L298NMotorPins _motor_a_pins;
    L298NMotorPins _motor_b_pins;

    /**
     * @brief Returns the pin configuration for a motor channel.
     *
     * @param motorid Motor channel identifier.
     *
     * @return Pointer to the selected motor pin configuration,
     *         or nullptr if motorid is invalid.
     */
    L298NMotorPins* get_motor_pins(uint8_t motorid);

    /**
     * @brief Writes the required GPIO states for a motor direction.
     *
     * @param pins GPIO configuration for the selected motor.
     * @param direction Desired motor direction or STOP state.
     */
    void write_direction(
        const L298NMotorPins& pins,
        l298n_dir direction
    );

    /**
     * @brief Configures one motor channel's GPIO pins.
     *
     * @param pins GPIO configuration for the selected motor.
     */
    void configure_motor_pins(const L298NMotorPins& pins);
};