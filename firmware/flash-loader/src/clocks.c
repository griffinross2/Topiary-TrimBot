#include "clocks.h"

#include "stm32h7xx_hal.h"
#include "system_stm32h7xx.h"

Status clocks_init()
{
    // Power and clock initialization
    // Power: LDO supply, VOS0 voltage scaling
    // Clock source: HSI
    // PLL1: P: 480 MHz, Q: 48 MHz, R: 480 MHz
    // QSPI: 240 MHz (HCLK3)

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
    
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();

    return STATUS_OK;
}