/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#pragma once

#include "gpio/gpio.h"
#include "board.h"

/**
 * Fast I/O interfaces for STM32
 * These use GPIO register access for fast port manipulation.
 */

// ------------------------
// Defines
// ------------------------

#define HIGH true
#define LOW  false

#define _BV32(b) (1UL << (b))

#ifndef PWM
  #define PWM OUTPUT
#endif

// #define _GET_MODE(IO)
// #define _SET_MODE(IO,M)         pinMode(IO, M)
// #define _SET_OUTPUT(IO)         pinMode(IO, OUTPUT)                               //!< Output Push Pull Mode & GPIO_NOPULL
// #define _SET_OUTPUT_OD(IO)      pinMode(IO, OUTPUT_OPEN_DRAIN)

#define WRITE(IO,V)             gpio_write(IO, (GpioValue)V)
#define READ(IO)                gpio_read(IO)
#define TOGGLE(IO)              gpio_toggle(IO)

#define OUT_WRITE(IO,V)         gpio_write(IO, (GpioValue)V)
#define OUT_WRITE_OD(IO,V)      do{ gpio_mode(IO, GPIO_OUTPUT_OD); gpio_write(IO, (GpioValue)V); }while(0)

#define SET_INPUT(IO)           gpio_mode(IO, GPIO_INPUT)
#define SET_INPUT_PULLUP(IO)    gpio_mode(IO, GPIO_INPUT_PULLUP)
#define SET_INPUT_PULLDOWN(IO)  gpio_mode(IO, GPIO_INPUT_PULLDOWN)
#define SET_OUTPUT(IO)          gpio_mode(IO, GPIO_OUTPUT)
#define SET_OUTPUT_OD(IO)       gpio_mode(IO, GPIO_OUTPUT_OD)
// #define SET_PWM(IO)             _SET_MODE(IO, PWM)

#define IS_INPUT(IO)
#define IS_OUTPUT(IO)

// #define PWM_PIN(P)              digitalPinHasPWM(P)
#define NO_COMPILE_TIME_PWM

// digitalRead/Write wrappers
#define extDigitalRead(IO)    gpio_read(IO)
#define extDigitalWrite(IO,V) gpio_write(IO, (GpioValue)V)
