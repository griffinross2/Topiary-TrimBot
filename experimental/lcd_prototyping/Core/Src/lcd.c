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
extern LTDC_HandleTypeDef hltdc;

LCD_Status lcd_init() {
	if(nt35510_init(&hdsi, GPIOH, GPIO_PIN_7) != NT35510_OK) {
		return LCD_ERROR;
	}
	nt35510_set_brightness(&hdsi, 200);
//	nt35510_all_pixels_on(&hdsi);
	nt35510_ram_write(&hdsi);
	printf("LTDC state: %d\n", hltdc.State);
	HAL_DSI_Refresh(&hdsi);

	*((volatile uint32_t*)0xC0800000) = 67;
	printf("%ld\n", *((volatile uint32_t*)0xC0800000));

//	 __HAL_LTDC_LAYER_ENABLE(&hltdc, LTDC_LAYER_1);
//	volatile uint32_t* framebuffer = (volatile uint32_t*)hltdc.LayerCfg[0].FBStartAdress;
//	for (int i = 0; i < 5000; i++) {
//		framebuffer[i] = 0x0000FF00;
//	}
//	HAL_DSI_Refresh(&hdsi);

	 printf("LTDC state: %d\n", hltdc.State);

	return LCD_OK;
}
