#include "gpio/gpio.h"

// Adapted from Purdue Space Program High Altitude, Originally written by
// Griffin Ross

#include "board.h"
#include "stm32h7xx_hal.h"

Status gpio_mode(uint8_t pin, GpioMode mode, GpioSpeed speed, uint32_t af)
{
    uint32_t gpio_pin = BOARD_GPIO_PIN(pin);
    GPIO_TypeDef *base = BOARD_GPIO_PORT(pin);
    GPIO_InitTypeDef conf = {
        .Pin = gpio_pin,
        .Mode = GPIO_MODE_INPUT,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW,
        .Alternate = 0,
    };

    // Speed
    switch (speed)
    {
    case GPIO_SPD_LOW:
        conf.Speed = GPIO_SPEED_FREQ_LOW;
        break;
    case GPIO_SPD_MEDIUM:
        conf.Speed = GPIO_SPEED_FREQ_MEDIUM;
        break;
    case GPIO_SPD_HIGH:
        conf.Speed = GPIO_SPEED_FREQ_HIGH;
        break;
    case GPIO_SPD_VERY_HIGH:
        conf.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        break;
    default:
        return STATUS_PARAMETER_ERROR;
    }

    // Mode, alternate
    switch (mode)
    {
    case GPIO_INPUT:
    case GPIO_INPUT_PULLDOWN:
    case GPIO_INPUT_PULLUP:
        conf.Mode = GPIO_MODE_INPUT;
        break;
    case GPIO_OUTPUT:
        conf.Mode = GPIO_MODE_OUTPUT_PP;
        break;
    case GPIO_OUTPUT_OD:
        conf.Mode = GPIO_MODE_OUTPUT_OD;
        break;
    case GPIO_OUTPUT_AF:
        conf.Mode = GPIO_MODE_AF_PP;
        conf.Alternate = af;
        break;
    case GPIO_OUTPUT_OD_AF:
        conf.Mode = GPIO_MODE_AF_OD;
        conf.Alternate = af;
        break;
    case GPIO_ANALOG:
        conf.Mode = GPIO_MODE_ANALOG;
        break;
    default:
        return STATUS_PARAMETER_ERROR;
    }

    // Pull
    switch (mode)
    {
    case GPIO_INPUT_PULLUP:
        conf.Pull = GPIO_PULLUP;
        break;
    case GPIO_INPUT_PULLDOWN:
        conf.Pull = GPIO_PULLDOWN;
        break;
    default:
        conf.Pull = GPIO_NOPULL;
        break;
    }

    HAL_GPIO_Init(base, &conf);
    return STATUS_OK;
}

Status gpio_write(uint8_t pin, GpioValue value)
{
    uint32_t gpio_pin = BOARD_GPIO_PIN(pin);
    GPIO_TypeDef *base = BOARD_GPIO_PORT(pin);
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
        current_mode != GPIO_MODE_OUTPUT_OD)
    {
        HAL_GPIO_Init(base, &conf);
    }
    HAL_GPIO_WritePin(base, gpio_pin, (GPIO_PinState)value);
    return STATUS_OK;
}

GpioValue gpio_read(uint8_t pin)
{
    uint32_t gpio_pin = BOARD_GPIO_PIN(pin);
    GPIO_TypeDef *base = BOARD_GPIO_PORT(pin);
    GPIO_InitTypeDef conf = {
        .Pin = gpio_pin,
        .Mode = GPIO_MODE_INPUT,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW,
        .Alternate = 0,
    };
    if ((BOARD_GPIO_PORT(pin)->MODER & BOARD_GPIO_MODER_MASK(pin)) >>
            BOARD_GPIO_MODER_POS(pin) !=
        GPIO_MODE_INPUT)
    {
        HAL_GPIO_Init(base, &conf);
    }
    uint32_t val = HAL_GPIO_ReadPin(base, gpio_pin);
    if (val == GPIO_HIGH)
    {
        return GPIO_HIGH;
    }
    else if (val == GPIO_LOW)
    {
        return GPIO_LOW;
    }
    else
    {
        return GPIO_ERR;
    }
}