#include "tilt_control.h"

TiltController::TiltController(L298N& driver1, L298N& driver2)
    : _driver1(driver1),
      _driver2(driver2)
{
}

void TiltController::init() {
    _driver1.init();
    _driver2.init();

    stop_all();
}

void TiltController::tilt(TiltDirection direction) {
    switch (direction)
    {
        case TiltDirection::LEFT:
            tilt_left();
            break;

        case TiltDirection::RIGHT:
            tilt_right();
            break;

        case TiltDirection::UP:
            tilt_up();
            break;

        case TiltDirection::DOWN:
            tilt_down();
            break;

        default:
            stop_all();
            break;
    }
}

void TiltController::tilt_left() {
    // Tilt toward LEFT:
    // lower the left side and raise the right side.
    lower_left_side();
    raise_right_side();
}

void TiltController::tilt_right() {
    // Tilt toward RIGHT:
    // raise the left side and lower the right side.
    raise_left_side();
    lower_right_side();
}

void TiltController::tilt_up() {
    // Tilt toward UP:
    // raise the bottom side and lower the top side.
    raise_bottom_side();
    lower_top_side();
}

void TiltController::tilt_down() {
    // Tilt toward DOWN:
    // raise the top side and lower the bottom side.
    raise_top_side();
    lower_bottom_side();
}

void TiltController::raise_all() {
    _driver1.set_motor(MOTOR_A, UP);   // Top-left
    _driver1.set_motor(MOTOR_B, UP);   // Bottom-left

    _driver2.set_motor(MOTOR_A, UP);   // Top-right
    _driver2.set_motor(MOTOR_B, UP);   // Bottom-right
}

void TiltController::lower_all() {
    _driver1.set_motor(MOTOR_A, DOWN); // Top-left
    _driver1.set_motor(MOTOR_B, DOWN); // Bottom-left

    _driver2.set_motor(MOTOR_A, DOWN); // Top-right
    _driver2.set_motor(MOTOR_B, DOWN); // Bottom-right
}

void TiltController::stop_all() {
    _driver1.stop_all();
    _driver2.stop_all();
}

void TiltController::raise_left_side() {
    _driver1.set_motor(MOTOR_A, UP); // Top-left
    _driver1.set_motor(MOTOR_B, UP); // Bottom-left
}

void TiltController::lower_left_side() {
    _driver1.set_motor(MOTOR_A, DOWN); // Top-left
    _driver1.set_motor(MOTOR_B, DOWN); // Bottom-left
}

void TiltController::raise_right_side() {
    _driver2.set_motor(MOTOR_A, UP); // Top-right
    _driver2.set_motor(MOTOR_B, UP); // Bottom-right
}

void TiltController::lower_right_side() {
    _driver2.set_motor(MOTOR_A, DOWN); // Top-right
    _driver2.set_motor(MOTOR_B, DOWN); // Bottom-right
}

void TiltController::raise_top_side() {
    _driver1.set_motor(MOTOR_A, UP); // Top-left
    _driver2.set_motor(MOTOR_A, UP); // Top-right
}

void TiltController::lower_top_side() {
    _driver1.set_motor(MOTOR_A, DOWN); // Top-left
    _driver2.set_motor(MOTOR_A, DOWN); // Top-right
}

void TiltController::raise_bottom_side() {
    _driver1.set_motor(MOTOR_B, UP); // Bottom-left
    _driver2.set_motor(MOTOR_B, UP); // Bottom-right
}

void TiltController::lower_bottom_side() {
    _driver1.set_motor(MOTOR_B, DOWN); // Bottom-left
    _driver2.set_motor(MOTOR_B, DOWN); // Bottom-right
}