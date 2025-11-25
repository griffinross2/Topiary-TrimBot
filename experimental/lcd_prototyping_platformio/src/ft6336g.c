#include "ft6336g.h"

#include "stm32f4xx_hal.h"

#define FT6336G_ADDRESS 0x70

static I2C_HandleTypeDef s_hi2c1;

static ft6336h_write(uint8_t reg, uint8_t *data, unsigned int len)
{
}

Status ft6336g_init()
{
    s_hi2c1.Instance = I2C1;
    s_hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    s_hi2c1.Init.ClockSpeed = 400000;
    s_hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    s_hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    s_hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    s_hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    s_hi2c1.Init.OwnAddress1 = 0;
    s_hi2c1.Init.OwnAddress2 = 0;

    if (HAL_I2C_Init(&s_hi2c1) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    return STATUS_OK;
}