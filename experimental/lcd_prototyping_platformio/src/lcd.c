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

#define LCD_RESET_PORT GPIOH
#define LCD_RESET_PIN GPIO_PIN_7

DSI_HandleTypeDef *hdsi;
LTDC_HandleTypeDef *hltdc;

static uint32_t __attribute__((section(".ext_ram"))) s_foreground_buffer[LCD_WIDTH * LCD_HEIGHT];

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

	HAL_LTDC_SetAddress(hltdc, (uint32_t)s_foreground_buffer, LTDC_LAYER_2);

	return STATUS_OK;
}

void lcd_set_background(const uint32_t *fb_address)
{
	HAL_DSI_Stop(hdsi);
	__HAL_LTDC_LAYER_DISABLE(hltdc, LTDC_LAYER_1);
	HAL_LTDC_SetAddress(hltdc, (uint32_t)fb_address, LTDC_LAYER_1);
	__HAL_LTDC_LAYER_ENABLE(hltdc, LTDC_LAYER_1);
	HAL_DSI_Start(hdsi);
	HAL_DSI_Refresh(hdsi);
}

void lcd_clear_foreground()
{
	memset(s_foreground_buffer, 0x00, sizeof(s_foreground_buffer));
	HAL_DSI_Refresh(hdsi);
}

void lcd_draw_rectangle(unsigned int x, unsigned int y, unsigned int w, unsigned int h, uint32_t color)
{
	for (unsigned int xi = x; xi < x + w; xi++)
	{
		for (unsigned int yi = y; yi < y + h; yi++)
		{
			s_foreground_buffer[yi + xi * LCD_HEIGHT] = color;
		}
	}
	HAL_DSI_Refresh(hdsi);
}