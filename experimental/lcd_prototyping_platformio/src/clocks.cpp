#include "clocks.h"

#include "stm32f4xx_hal.h"
#include "system_stm32f4xx.h"

Status clocks_init() {
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitTypeDef RCC_OscInitStruct = {
        .OscillatorType = RCC_OSCILLATORTYPE_HSE,
        .HSEState = RCC_HSE_ON,
        .LSEState = RCC_LSE_OFF,
        .HSIState = RCC_HSI_OFF,
        .HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT,
        .LSIState = RCC_LSI_OFF,
        .PLL =
            {
                .PLLState = RCC_PLL_ON,
                .PLLSource = RCC_PLLSOURCE_HSE,
                .PLLM = 4,
                .PLLN = 180,
                .PLLP = RCC_PLLP_DIV2,
                .PLLQ = 4,
                .PLLR = 6,
            },
    };

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        return STATUS_ERROR;
    }

    if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
        return STATUS_ERROR;
    }

    RCC_ClkInitTypeDef RCC_ClkInitStruct = {
        .ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                     RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2,
        .SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK,
        .AHBCLKDivider = RCC_SYSCLK_DIV1,
        .APB1CLKDivider = RCC_HCLK_DIV4,
        .APB2CLKDivider = RCC_HCLK_DIV2,
    };

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
        return STATUS_ERROR;
    }

    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {
        .PeriphClockSelection =
            RCC_PERIPHCLK_LTDC | RCC_PERIPHCLK_CLK48 | RCC_PERIPHCLK_SDIO,
        .PLLI2S =
            {
                .PLLI2SN = 0,
                .PLLI2SR = 0,
                .PLLI2SQ = 0,
            },
        .PLLSAI =
            {
                .PLLSAIN = 144,
                .PLLSAIP = RCC_PLLSAIP_DIV6,
                .PLLSAIQ = 2,
                .PLLSAIR = 2,
            },
        .PLLI2SDivQ = 1,
        .PLLSAIDivQ = 1,
        .PLLSAIDivR = RCC_PLLSAIDIVR_8,
        .RTCClockSelection = RCC_RTCCLKSOURCE_LSE,
        .TIMPresSelection = RCC_TIMPRES_DESACTIVATED,
        .Clk48ClockSelection = RCC_CLK48CLKSOURCE_PLLSAIP,
        .SdioClockSelection = RCC_SDIOCLKSOURCE_CLK48,
    };
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        return STATUS_ERROR;
    }

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOI_CLK_ENABLE();
    __HAL_RCC_GPIOJ_CLK_ENABLE();
    __HAL_RCC_GPIOK_CLK_ENABLE();

    return STATUS_OK;
}