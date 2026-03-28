#ifndef GPIO_H
#define GPIO_H

// Adapted from Purdue Space Program High Altitude, Originally written by
// Griffin Ross

#include <stdint.h>

#include "status.h"

typedef enum
{
    GPIO_INPUT = 0,
    GPIO_OUTPUT = 1,
    GPIO_OUTPUT_AF = 2,
    GPIO_OUTPUT_OD = 3,
    GPIO_OUTPUT_OD_AF = 4,
    GPIO_INPUT_PULLUP = 5,
    GPIO_INPUT_PULLDOWN = 6,
    GPIO_ANALOG = 7,
} GpioMode;

typedef enum
{
    GPIO_ERR = -1,
    GPIO_LOW = 0,
    GPIO_HIGH = 1,
} GpioValue;

typedef enum
{
    GPIO_SPD_LOW = 0,
    GPIO_SPD_MEDIUM = 1,
    GPIO_SPD_HIGH = 2,
    GPIO_SPD_VERY_HIGH = 3,
} GpioSpeed;

Status
gpio_mode(uint8_t pin, GpioMode mode, GpioSpeed speed = GPIO_SPD_LOW, uint32_t af = 0);

Status gpio_write(uint8_t pin, GpioValue value);

GpioValue gpio_read(uint8_t pin);

#endif // GPIO_H
