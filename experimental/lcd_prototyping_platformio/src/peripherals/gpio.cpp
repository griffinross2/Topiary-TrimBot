#include "gpio/gpio.h"

#include "board.h"
#include "stm32f4xx_hal.h"

Status gpio_mode(uint8_t pin, GpioMode mode) {
    uint32_t gpio_pin = BOARD_GPIO_PIN(pin);
    GPIO_TypeDef* base = BOARD_GPIO_PORT(pin);
    GPIO_InitTypeDef conf = {
        .Pin = gpio_pin,
        .Mode = GPIO_MODE_INPUT,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW,
        .Alternate = 0,
    };
    if ((mode == GPIO_INPUT) | (mode == GPIO_INPUT_PULLDOWN) |
        (mode == GPIO_INPUT_PULLUP))
        conf.Mode = GPIO_MODE_INPUT;
    else if (mode == GPIO_OUTPUT)
        conf.Mode = GPIO_MODE_OUTPUT_OD;
    else if (mode == GPIO_OUTPUT_OD)
        conf.Mode = GPIO_MODE_OUTPUT_OD;
    else if (mode == GPIO_ANALOG)
        conf.Mode = GPIO_MODE_ANALOG;
    if (mode == GPIO_INPUT_PULLUP)
        conf.Pull = GPIO_PULLUP;
    else if (mode == GPIO_INPUT_PULLDOWN)
        conf.Pull = GPIO_PULLDOWN;
    else
        conf.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(base, &conf);
    return STATUS_OK;
}

Status gpio_write(uint8_t pin, GpioValue value) {
    uint32_t gpio_pin = BOARD_GPIO_PIN(pin);
    GPIO_TypeDef* base = BOARD_GPIO_PORT(pin);
    GPIO_InitTypeDef conf = {
        .Pin = gpio_pin,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW,
        .Alternate = 0,
    };
    uint32_t current_mode =
        (BOARD_GPIO_PORT(pin)->MODER & BOARD_GPIO_MODER_MASK(pin)) >>
        BOARD_GPIO_MODER_POS(pin);
    if (current_mode != GPIO_MODE_OUTPUT_PP &&
        current_mode != GPIO_MODE_OUTPUT_OD) {
        HAL_GPIO_Init(base, &conf);
    }
    HAL_GPIO_WritePin(base, gpio_pin, (GPIO_PinState)value);
    return STATUS_OK;
}

GpioValue gpio_read(uint8_t pin) {
    uint32_t gpio_pin = BOARD_GPIO_PIN(pin);
    GPIO_TypeDef* base = BOARD_GPIO_PORT(pin);
    GPIO_InitTypeDef conf = {
        .Pin = gpio_pin,
        .Mode = GPIO_MODE_INPUT,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW,
        .Alternate = 0,
    };
    if ((BOARD_GPIO_PORT(pin)->MODER & BOARD_GPIO_MODER_MASK(pin)) >>
            BOARD_GPIO_MODER_POS(pin) !=
        GPIO_MODE_INPUT) {
        HAL_GPIO_Init(base, &conf);
    }
    uint32_t val = HAL_GPIO_ReadPin(base, gpio_pin);
    if (val == GPIO_HIGH) {
        return GPIO_HIGH;
    } else if (val == GPIO_LOW) {
        return GPIO_LOW;
    } else {
        return GPIO_ERR;
    }
}