#include "clocks.h"

#include "stm32h7xx_hal.h"
#include "system_stm32h7xx.h"

Status clocks_init()
{
    // Power and clock initialization
    // Power: LDO supply, VOS0 voltage scaling
    // Clock source: HSE in bypass
    // PLL1: P: 480 MHz, Q: 48 MHz, R: 480 MHz
    // I2C1,2,3: 120 MHz (PCLK1)
    // USB: 48 MHz (PLL1Q)
    // SDMMC: 48 MHz (PLL1Q)
    // QSPI: 240 MHz (HCLK3)
    // LTDC: 33.33 MHz (PLL3R)

    __HAL_RCC_SYSCFG_CLK_ENABLE();
    HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE0);

    while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY))
    {
    }

    // Oscillator and PLL initialization
    // Start with HSI
    RCC_OscInitTypeDef osc_init = {};
    osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    osc_init.HSIState = RCC_HSI_DIV2;
    osc_init.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    osc_init.PLL.PLLState = RCC_PLL_ON;
    osc_init.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    osc_init.PLL.PLLM = 8;
    osc_init.PLL.PLLN = 240;
    osc_init.PLL.PLLP = 2;
    osc_init.PLL.PLLQ = 20;
    osc_init.PLL.PLLR = 2;
    osc_init.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
    osc_init.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    osc_init.PLL.PLLFRACN = 0;
    if (HAL_RCC_OscConfig(&osc_init) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    // Manually enable HSE (no HAL) to avoid timeout in HSERDY check
    RCC->CR |= RCC_CR_HSEBYP;
    RCC->CR |= RCC_CR_HSEON;
    HAL_Delay(100);
    RCC->PLLCKSELR |= RCC_PLLCKSELR_PLLSRC_HSE;

    // CPU and Bus clock initialization
    RCC_ClkInitTypeDef clk_init = {};
    clk_init.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
    clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk_init.SYSCLKDivider = RCC_SYSCLK_DIV1;
    clk_init.AHBCLKDivider = RCC_HCLK_DIV2;
    clk_init.APB3CLKDivider = RCC_APB3_DIV2;
    clk_init.APB1CLKDivider = RCC_APB1_DIV2;
    clk_init.APB2CLKDivider = RCC_APB2_DIV2;
    clk_init.APB4CLKDivider = RCC_APB4_DIV2;

    if (HAL_RCC_ClockConfig(&clk_init, FLASH_LATENCY_4) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    // Peripheral clocks initialization
    RCC_PeriphCLKInitTypeDef pclk_init = {};
    pclk_init.PeriphClockSelection = RCC_PERIPHCLK_SDMMC | RCC_PERIPHCLK_LTDC;
    pclk_init.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL;
    pclk_init.PLL3.PLL3M = 32;
    pclk_init.PLL3.PLL3N = 200;
    pclk_init.PLL3.PLL3P = 2;
    pclk_init.PLL3.PLL3Q = 2;
    pclk_init.PLL3.PLL3R = 6;
    pclk_init.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_0;
    pclk_init.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
    pclk_init.PLL3.PLL3FRACN = 0;

    if (HAL_RCCEx_PeriphCLKConfig(&pclk_init) != HAL_OK)
    {
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

    __HAL_RCC_HSEM_CLK_ENABLE();

    return STATUS_OK;
}