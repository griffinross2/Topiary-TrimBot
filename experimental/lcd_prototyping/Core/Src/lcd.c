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

NT35510_Object_t lcdObj;
NT35510_IO_t lcdIO;

static int32_t lcd_io_read(uint16_t ChannelNbr, uint16_t Reg, uint8_t *pData, uint16_t Size);
static int32_t lcd_io_write(uint16_t ChannelNbr, uint16_t Reg, uint8_t *pData, uint16_t Size);

void lcd_init() {
	GPIO_InitTypeDef lcd_reset = {
			.Pin = LCD_RESET_PIN,
			.Mode = GPIO_MODE_OUTPUT_PP,
			.Pull = GPIO_NOPULL,
			.Speed = GPIO_SPEED_FREQ_LOW,
	};

	lcdIO.Address = 0;
	lcdIO.GetTick = HAL_GetTick;
	lcdIO.ReadReg = lcd_io_read;
	lcdIO.WriteReg = lcd_io_write;
	NT35510_RegisterBusIO(&lcdObj, &lcdIO);


	HAL_GPIO_Init(LCD_RESET_PORT, &lcd_reset);
	HAL_GPIO_WritePin(LCD_RESET_PORT, LCD_RESET_PIN, GPIO_PIN_SET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(LCD_RESET_PORT, LCD_RESET_PIN, GPIO_PIN_RESET);
	HAL_Delay(20);
	HAL_GPIO_WritePin(LCD_RESET_PORT, LCD_RESET_PIN, GPIO_PIN_SET);
	HAL_Delay(50);

	HAL_DSI_Start(&hdsi);

	NT35510_Init(&lcdObj, NT35510_FORMAT_RGB888, NT35510_ORIENTATION_LANDSCAPE);

	uint32_t id = 0;
	NT35510_ReadID(&lcdObj, &id);

	printf("%ld\n", id);
}

static int32_t lcd_io_write(uint16_t ChannelNbr, uint16_t Reg, uint8_t *pData, uint16_t Size)
{
  int32_t ret = NT35510_OK;

  if(Size <= 1U)
  {
    if(HAL_DSI_ShortWrite(&hdsi, ChannelNbr, DSI_DCS_SHORT_PKT_WRITE_P1, Reg, (uint32_t)pData[Size]) != HAL_OK)
    {
      ret = NT35510_ERROR;
    }
  }
  else
  {
    if(HAL_DSI_LongWrite(&hdsi, ChannelNbr, DSI_DCS_LONG_PKT_WRITE, Size, (uint32_t)Reg, pData) != HAL_OK)
    {
      ret = NT35510_ERROR;
    }
  }

  return ret;
}

static int32_t lcd_io_read(uint16_t ChannelNbr, uint16_t Reg, uint8_t *pData, uint16_t Size)
{
  int32_t ret = NT35510_OK;

  if(HAL_DSI_Read(&hdsi, ChannelNbr, pData, Size, DSI_DCS_SHORT_PKT_READ, Reg, pData) != HAL_OK)
  {
    ret = NT35510_ERROR;
  }

  return ret;
}
