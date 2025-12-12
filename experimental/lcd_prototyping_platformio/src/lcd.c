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
#include "ft6336g.h"

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

	GPIO_InitTypeDef lcd_reset = {
		.Pin = LCD_RESET_PIN,
		.Mode = GPIO_MODE_OUTPUT_PP,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_FREQ_LOW,
	};

	// Reset the LCD and touchscreen
	HAL_GPIO_Init(LCD_RESET_PORT, &lcd_reset);
	HAL_GPIO_WritePin(LCD_RESET_PORT, LCD_RESET_PIN, GPIO_PIN_SET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(LCD_RESET_PORT, LCD_RESET_PIN, GPIO_PIN_RESET);
	HAL_Delay(20);
	HAL_GPIO_WritePin(LCD_RESET_PORT, LCD_RESET_PIN, GPIO_PIN_SET);
	HAL_Delay(20);

	// LCD init
	if (nt35510_init(hdsi) != STATUS_OK)
	{
		return STATUS_ERROR;
	}
	nt35510_set_brightness(hdsi, 200);

	HAL_DSI_Refresh(hdsi);

	HAL_LTDC_SetAddress(hltdc, (uint32_t)s_foreground_buffer, LTDC_LAYER_2);

	// Touchscreen init
	// Give some time after reset for the TS driver to become ready
	HAL_Delay(300);

	if (ft6336g_init() != STATUS_OK)
	{
		return STATUS_ERROR;
	}

	return STATUS_OK;
}

uint32_t *lcd_get_framebuffer()
{
	return s_foreground_buffer;
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

void lcd_draw_circle(unsigned int x, unsigned int y, unsigned int r, uint32_t color)
{
	for (unsigned int xi = x - r; xi < x + r; xi++)
	{
		for (unsigned int yi = y - r; yi < y + r; yi++)
		{
			int dx = (int)xi - (int)x;
			int dy = (int)yi - (int)y;

			if (dx * dx + dy * dy < r * r)
			{
				s_foreground_buffer[yi + xi * LCD_HEIGHT] = color;
			}
		}
	}
	HAL_DSI_Refresh(hdsi);
}

void lcd_copy_background_to_foreground(const uint32_t *fb_address)
{
	if (fb_address != NULL)
	{
		memcpy(s_foreground_buffer, fb_address, sizeof(s_foreground_buffer));
	}
	else
	{
		// Wait until VSYNC to avoid stepping on the LTDC
		lcd_wait_for_vsync();

		memcpy(s_foreground_buffer, (uint32_t *)hltdc->LayerCfg[0].FBStartAdress, sizeof(s_foreground_buffer));
	}
	HAL_DSI_Refresh(hdsi);
}

void lcd_set_foreground_alpha(uint8_t alpha)
{
	HAL_LTDC_SetAlpha(hltdc, alpha, LTDC_LAYER_2);
	HAL_DSI_Refresh(hdsi);
}

void lcd_set_foreground_visibility(bool visible)
{
	if (visible)
	{
		__HAL_LTDC_LAYER_ENABLE(hltdc, LTDC_LAYER_2);
	}
	else
	{
		__HAL_LTDC_LAYER_DISABLE(hltdc, LTDC_LAYER_2);
	}
	HAL_DSI_Refresh(hdsi);
}

void lcd_wait_for_vsync()
{
	while ((hltdc->Instance->CDSR & LTDC_CDSR_VSYNCS) == 0)
	{
	}
}

void lcd_touch_irq()
{
	ft6336g_irq();

	FT6336G_TouchEvent event = ft6336g_get_touch_event(0);
	if (event == FT6336G_TOUCH_EVENT_DOWN || event == FT6336G_TOUCH_EVENT_CONTACT)
	{
		int x, y, weight;
		if (ft6336g_read_pos(&y, &x, &weight, 0) == STATUS_OK)
		{
			lcd_draw_circle(x, y, 5, 0xFFFF0000);
		}
	}
}