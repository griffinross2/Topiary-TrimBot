#include "nt35510.h"

#include <stdio.h>

void nt35510_write(DSI_HandleTypeDef *hdsi, uint8_t reg, uint32_t len, uint8_t *data)
{
	if (len <= 1)
	{
		if (data == NULL)
		{
			HAL_DSI_ShortWrite(hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, reg, 0);
		}
		else
		{
			HAL_DSI_ShortWrite(hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, reg, data[0]);
		}
	}
	else
	{
		HAL_DSI_LongWrite(hdsi, 0, DSI_DCS_LONG_PKT_WRITE, len, reg, data);
	}
}

void nt35510_read(DSI_HandleTypeDef *hdsi, uint8_t reg, uint32_t len, uint8_t *data)
{
	HAL_DSI_Read(hdsi, 0, data, len, DSI_DCS_SHORT_PKT_READ, reg, data);
}

Status nt35510_init(DSI_HandleTypeDef *hdsi, GPIO_TypeDef *lcd_reset_port, uint32_t lcd_reset_pin)
{
	GPIO_InitTypeDef lcd_reset = {
		.Pin = lcd_reset_pin,
		.Mode = GPIO_MODE_OUTPUT_PP,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_FREQ_LOW,
	};

	// Reset the LCD
	HAL_GPIO_Init(lcd_reset_port, &lcd_reset);
	HAL_GPIO_WritePin(lcd_reset_port, lcd_reset_pin, GPIO_PIN_SET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(lcd_reset_port, lcd_reset_pin, GPIO_PIN_RESET);
	HAL_Delay(20);
	HAL_GPIO_WritePin(lcd_reset_port, lcd_reset_pin, GPIO_PIN_SET);
	HAL_Delay(50);

	HAL_DSI_Start(hdsi);

	uint8_t id = 0;
	nt35510_read(hdsi, NT35510_CMD_RDID2, 1, &id);
	printf("LCD Driver Version: 0x%x\n", id);
	if (id == 0)
	{
		return STATUS_ERROR;
	}

	nt35510_write(hdsi, NT35510_CMD_SLPOUT, 0, NULL); // Exit sleep mode
	HAL_Delay(20);
	nt35510_write(hdsi, NT35510_CMD_DISPON, 0, NULL); // Display on

	uint8_t reg_val = 0x2C;
	nt35510_write(hdsi, NT35510_CMD_WRCTRLD, 1, &reg_val); // Backlight on

	return STATUS_OK;
}

void nt35510_set_brightness(DSI_HandleTypeDef *hdsi, uint8_t brightness)
{
	nt35510_write(hdsi, NT35510_CMD_WRDISBV, 1, &brightness);
}

void nt35510_all_pixels_on(DSI_HandleTypeDef *hdsi)
{
	nt35510_write(hdsi, NT35510_CMD_ALLPON, 0, NULL);
}

void nt35510_ram_write(DSI_HandleTypeDef *hdsi)
{
	nt35510_write(hdsi, NT35510_CMD_RAMWR, 0, NULL);
}

void nt35510_normal_mode(DSI_HandleTypeDef *hdsi)
{
	nt35510_write(hdsi, NT35510_CMD_NORON, 0, NULL);
}

void nt35510_partial_mode(DSI_HandleTypeDef *hdsi)
{
	nt35510_write(hdsi, NT35510_CMD_PTLON, 0, NULL);
}

void nt35510_nop(DSI_HandleTypeDef *hdsi)
{
	nt35510_write(hdsi, NT35510_CMD_NOP, 0, NULL);
}

void nt35510_madctl(DSI_HandleTypeDef *hdsi, uint8_t val)
{
	nt35510_write(hdsi, NT35510_CMD_MADCTL, 1, &val);
}