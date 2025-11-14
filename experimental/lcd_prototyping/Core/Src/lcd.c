/*
 * lcd.c
 *
 *  Created on: Nov 11, 2025
 *      Author: griff
 */

#include "lcd.h"

#include "stm32f4xx_hal.h"
#include "nt35510.h"
#include <stdio.h>

#define LCD_RESET_PORT GPIOH
#define LCD_RESET_PIN GPIO_PIN_7

extern DSI_HandleTypeDef hdsi;

LCD_Status lcd_init() {
	if(nt35510_init(&hdsi, GPIOH, GPIO_PIN_7) != NT35510_OK) {
		return LCD_ERROR;
	}
	nt35510_set_brightness(&hdsi, 200);
//	nt35510_all_pixels_on(&hdsi);

	return LCD_OK;
}
