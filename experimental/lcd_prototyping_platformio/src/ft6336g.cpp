#include "ft6336g.h"

#include <string.h>
#include <stdio.h>

#include "stm32f4xx_hal.h"

#define FT6336G_ADDRESS 0x70

static I2C_HandleTypeDef s_hi2c1;
EXTI_HandleTypeDef g_hexti;

static Status ft6336g_write(uint8_t reg, uint8_t* data, unsigned int len) {
    uint8_t tx_buf[len + 1];
    tx_buf[0] = reg;
    memcpy(tx_buf + 1, data, len);

    if (HAL_I2C_Master_Transmit(&s_hi2c1, FT6336G_ADDRESS, tx_buf, len, 100) !=
        HAL_OK) {
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

static Status ft6336g_read(uint8_t reg, uint8_t* data, unsigned int len) {
    if (HAL_I2C_Master_Transmit(&s_hi2c1, FT6336G_ADDRESS, &reg, 1, 100) !=
        HAL_OK) {
        return STATUS_ERROR;
    }

    if (HAL_I2C_Master_Receive(&s_hi2c1, FT6336G_ADDRESS, data, len, 100) !=
        HAL_OK) {
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

Status ft6336g_init() {
    __HAL_RCC_I2C1_CLK_ENABLE();
    __HAL_RCC_I2C1_FORCE_RESET();
    __HAL_RCC_I2C1_RELEASE_RESET();

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    s_hi2c1.Instance = I2C1;
    s_hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    s_hi2c1.Init.ClockSpeed = 400000;
    s_hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    s_hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    s_hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    s_hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    s_hi2c1.Init.OwnAddress1 = 0;
    s_hi2c1.Init.OwnAddress2 = 0;

    if (HAL_I2C_Init(&s_hi2c1) != HAL_OK) {
        return STATUS_ERROR;
    }

    uint8_t id = 0x00;
    if (ft6336g_read(FT6336G_FOCALTECH_ID, &id, 1) != STATUS_OK) {
        return STATUS_ERROR;
    }

    // Should be 0x11
    if (id != 0x11) {
        return STATUS_ERROR;
    }

    // Configure touch interrupt
    g_hexti.Line = EXTI_LINE_5;
    EXTI_ConfigTypeDef exti_config = {
        .Line = EXTI_LINE_5,
        .Mode = EXTI_MODE_INTERRUPT,
        .Trigger = EXTI_TRIGGER_FALLING,
        .GPIOSel = EXTI_GPIOJ,
    };
    HAL_EXTI_SetConfigLine(&g_hexti, &exti_config);

    NVIC_SetPriority(EXTI9_5_IRQn, 1);
    NVIC_EnableIRQ(EXTI9_5_IRQn);

    return STATUS_OK;
}

void ft6336g_irq() {}

FT6336G_TouchEvent ft6336g_get_touch_event(int touch_point_index) {
    if (touch_point_index > 1 || touch_point_index < 0) {
        return FT6336G_TOUCH_EVENT_NONE;
    }

    uint8_t reg = touch_point_index ? FT6336G_P2_XH : FT6336G_P1_XH;

    uint8_t event;
    if (ft6336g_read(reg, &event, 1) != STATUS_OK) {
        return FT6336G_TOUCH_EVENT_NONE;
    }

    return (FT6336G_TouchEvent)((event >> 6) & 0x03);
}

Status ft6336g_read_pos(int* x, int* y, int* weight, int touch_point_index) {
    if (touch_point_index > 1 || touch_point_index < 0) {
        return STATUS_ERROR;
    }

    uint8_t base_reg = touch_point_index ? FT6336G_P2_XH : FT6336G_P1_XH;
    uint8_t data[5];
    if (ft6336g_read(base_reg, data, 5) != STATUS_OK) {
        return STATUS_ERROR;
    }

    *x = ((data[0] & 0x0F) << 8) | data[1];
    *y = ((data[2] & 0x0F) << 8) | data[3];
    *weight = data[3];

    return STATUS_OK;
}