#include "l298n.h"

L298N::L298N(
    const L298NMotorPins& motor_a_pins,
    const L298NMotorPins& motor_b_pins
)
    : _motor_a_pins(motor_a_pins),
      _motor_b_pins(motor_b_pins)
{
}

void L298N::init()
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    configure_motor_pins(_motor_a_pins);
    configure_motor_pins(_motor_b_pins);

    stop_all();
}

void L298N::set_motor(uint8_t motorid, l298n_dir direction)
{
    L298NMotorPins* pins = get_motor_pins(motorid);

    if (pins == nullptr)
    {
        return;
    }

    write_direction(*pins, direction);
}

void L298N::stop_motor(uint8_t motorid)
{
    set_motor(motorid, STOP);
}

void L298N::stop_all()
{
    write_direction(_motor_a_pins, STOP);
    write_direction(_motor_b_pins, STOP);
}

L298NMotorPins* L298N::get_motor_pins(uint8_t motorid)
{
    switch (motorid)
    {
        case MOTOR_A:
            return &_motor_a_pins;

        case MOTOR_B:
            return &_motor_b_pins;

        default:
            return nullptr;
    }
}

void L298N::write_direction(
    const L298NMotorPins& pins,
    l298n_dir direction
)
{
    GPIO_PinState input_a_state = GPIO_PIN_RESET;
    GPIO_PinState input_b_state = GPIO_PIN_RESET;

    switch (direction)
    {
        case UP:
            input_a_state = GPIO_PIN_SET;
            input_b_state = GPIO_PIN_RESET;
            break;

        case DOWN:
            input_a_state = GPIO_PIN_RESET;
            input_b_state = GPIO_PIN_SET;
            break;

        case STOP:
        default:
            input_a_state = GPIO_PIN_RESET;
            input_b_state = GPIO_PIN_RESET;
            break;
    }

    HAL_GPIO_WritePin(
        pins.input_a_port,
        pins.input_a_pin,
        input_a_state
    );

    HAL_GPIO_WritePin(
        pins.input_b_port,
        pins.input_b_pin,
        input_b_state
    );
}

void L298N::configure_motor_pins(
    const L298NMotorPins& pins
)
{
    GPIO_InitTypeDef gpio_init{};

    gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_LOW;

    gpio_init.Pin = pins.input_a_pin;
    HAL_GPIO_Init(pins.input_a_port, &gpio_init);

    gpio_init.Pin = pins.input_b_pin;
    HAL_GPIO_Init(pins.input_b_port, &gpio_init);

    HAL_GPIO_WritePin(
        pins.input_a_port,
        pins.input_a_pin,
        GPIO_PIN_RESET
    );

    HAL_GPIO_WritePin(
        pins.input_b_port,
        pins.input_b_pin,
        GPIO_PIN_RESET
    );
}