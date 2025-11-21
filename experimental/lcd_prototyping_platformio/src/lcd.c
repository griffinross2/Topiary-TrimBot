/*
 * lcd.c
 *
 *  Created on: Nov 11, 2025
 *      Author: griff
 */

#include "lcd.h"

#include <stdio.h>
#include <string.h>

#include "ltdc_dsi.h"
#include "nt35510.h"
#include "images/test.h"
#include "images/squares.h"

#define LCD_RESET_PORT GPIOH
#define LCD_RESET_PIN GPIO_PIN_7

DSI_HandleTypeDef *hdsi;
LTDC_HandleTypeDef *hltdc;

Status lcd_init()
{
	ltdc_dsi_init();
	hltdc = ltdc_get_handle();
	hdsi = dsi_get_handle();

	if (nt35510_init(hdsi, GPIOH, GPIO_PIN_7) != STATUS_OK)
	{
		return STATUS_ERROR;
	}
	nt35510_set_brightness(hdsi, 200);
	HAL_DSI_Refresh(hdsi);

	__HAL_LTDC_LAYER_DISABLE(hltdc, LTDC_LAYER_1);
	HAL_LTDC_SetAddress(hltdc, (uint32_t)TEST, LTDC_LAYER_1);
	__HAL_LTDC_LAYER_ENABLE(hltdc, LTDC_LAYER_1);

	HAL_DSI_Refresh(hdsi);

	return STATUS_OK;
}
