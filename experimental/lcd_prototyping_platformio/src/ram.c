#include "ram.h"

#include "stm32f4xx_hal.h"

SDRAM_HandleTypeDef g_hsdram1;

Status ram_init()
{
    __HAL_RCC_FMC_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_0 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_11 | GPIO_PIN_14 | GPIO_PIN_7 | GPIO_PIN_10 | GPIO_PIN_12 | GPIO_PIN_15 | GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_15 | GPIO_PIN_8 | GPIO_PIN_1 | GPIO_PIN_0 | GPIO_PIN_5 | GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_15 | GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_9 | GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_5 | GPIO_PIN_7 | GPIO_PIN_10 | GPIO_PIN_6 | GPIO_PIN_1 | GPIO_PIN_9 | GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_12 | GPIO_PIN_15 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_15 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_12 | GPIO_PIN_9 | GPIO_PIN_11 | GPIO_PIN_8 | GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    FMC_SDRAM_TimingTypeDef SdramTiming = {0};

    g_hsdram1.Instance = FMC_SDRAM_DEVICE;
    g_hsdram1.Init.SDBank = FMC_SDRAM_BANK1;
    g_hsdram1.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_8;
    g_hsdram1.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_12;
    g_hsdram1.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_32;
    g_hsdram1.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
    g_hsdram1.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_2;
    g_hsdram1.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
    g_hsdram1.Init.SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2;
    g_hsdram1.Init.ReadBurst = FMC_SDRAM_RBURST_ENABLE;
    g_hsdram1.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_0;

    SdramTiming.LoadToActiveDelay = 2;
    SdramTiming.ExitSelfRefreshDelay = 7;
    SdramTiming.SelfRefreshTime = 4;
    SdramTiming.RowCycleDelay = 7;
    SdramTiming.WriteRecoveryTime = 3;
    SdramTiming.RPDelay = 2;
    SdramTiming.RCDDelay = 16;

    if (HAL_SDRAM_Init(&g_hsdram1, &SdramTiming) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    // Follow initialization sequence per datasheet
    FMC_SDRAM_CommandTypeDef Command;

    Command.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
    Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber = 1;
    Command.ModeRegisterDefinition = 0;
    HAL_SDRAM_SendCommand(&g_hsdram1, &Command, 100);

    HAL_Delay(1);

    Command.CommandMode = FMC_SDRAM_CMD_PALL;
    HAL_SDRAM_SendCommand(&g_hsdram1, &Command, 100);

    Command.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
    Command.AutoRefreshNumber = 2;
    HAL_SDRAM_SendCommand(&g_hsdram1, &Command, 100);

    Command.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
    Command.ModeRegisterDefinition = (uint32_t)0 | 0 << 3 | 2 << 4 | 0 << 7 | 1 << 9;
    Command.AutoRefreshNumber = 1;
    HAL_SDRAM_SendCommand(&g_hsdram1, &Command, 100);

    HAL_SDRAM_ProgramRefreshRate(&g_hsdram1, 332);

    return STATUS_OK;
}