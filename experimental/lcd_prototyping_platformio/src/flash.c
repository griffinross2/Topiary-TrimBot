#include "flash.h"

#include "stm32f4xx_hal.h"

#include <stdio.h>

QSPI_HandleTypeDef hqspi;

static const QSPI_CommandTypeDef s_qspi_default_cmd_1_line = {
    .Address = 0,
    .AddressMode = QSPI_ADDRESS_1_LINE,
    .AddressSize = QSPI_ADDRESS_32_BITS,
    .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
    .AlternateBytes = 0,
    .AlternateBytesSize = 0,
    .DataMode = QSPI_DATA_1_LINE,
    .DdrHoldHalfCycle = 0,
    .DdrMode = QSPI_DDR_MODE_DISABLE,
    .DummyCycles = 0,
    .Instruction = 0,
    .InstructionMode = QSPI_INSTRUCTION_1_LINE,
    .NbData = 0,
    .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,
};

static const QSPI_CommandTypeDef s_qspi_default_cmd_2_lines = {
    .Address = 0,
    .AddressMode = QSPI_ADDRESS_2_LINES,
    .AddressSize = QSPI_ADDRESS_32_BITS,
    .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
    .AlternateBytes = 0,
    .AlternateBytesSize = 0,
    .DataMode = QSPI_DATA_2_LINES,
    .DdrHoldHalfCycle = 0,
    .DdrMode = QSPI_DDR_MODE_DISABLE,
    .DummyCycles = 0,
    .Instruction = 0,
    .InstructionMode = QSPI_INSTRUCTION_2_LINES,
    .NbData = 0,
    .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,
};

static const QSPI_CommandTypeDef s_qspi_default_cmd_4_lines = {
    .Address = 0,
    .AddressMode = QSPI_ADDRESS_4_LINES,
    .AddressSize = QSPI_ADDRESS_32_BITS,
    .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
    .AlternateBytes = 0,
    .AlternateBytesSize = 0,
    .DataMode = QSPI_DATA_4_LINES,
    .DdrHoldHalfCycle = 0,
    .DdrMode = QSPI_DDR_MODE_DISABLE,
    .DummyCycles = 0,
    .Instruction = 0,
    .InstructionMode = QSPI_INSTRUCTION_4_LINES,
    .NbData = 0,
    .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,
};

static int qspi_mode = 0; // SPI

static QSPI_CommandTypeDef flash_get_default_cmd()
{
    switch (qspi_mode)
    {
    case 0:
        return s_qspi_default_cmd_1_line;
        break;
    case 1:
        return s_qspi_default_cmd_2_lines;
        break;
    case 2:
        return s_qspi_default_cmd_4_lines;
        break;
    }

    return s_qspi_default_cmd_1_line;
}

static void flash_wen()
{
    QSPI_CommandTypeDef cmd = flash_get_default_cmd();
    cmd.Instruction = 0x06;
    cmd.NbData = 0;
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.DataMode = QSPI_DATA_NONE;
    HAL_QSPI_Command(&hqspi, &cmd, 100);
}

static void flash_wel()
{
    flash_wen();

    QSPI_CommandTypeDef cmd = flash_get_default_cmd();
    cmd.Instruction = 0x01;
    cmd.NbData = 1;
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.DataMode = QSPI_DATA_1_LINE;
    uint8_t reg = 0x02;
    HAL_QSPI_Command(&hqspi, &cmd, 100);
    HAL_QSPI_Transmit(&hqspi, &reg, 100);
}

static void flash_set_qspi_mode()
{
    flash_wen();

    QSPI_CommandTypeDef cmd = flash_get_default_cmd();
    cmd.Instruction = 0x61;
    cmd.NbData = 1;
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    uint8_t reg = 0x7F;
    HAL_QSPI_Command(&hqspi, &cmd, 100);
    HAL_QSPI_Transmit(&hqspi, &reg, 100);

    qspi_mode = 2;
}

Status flash_init()
{
    __HAL_RCC_QSPI_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_QSPI;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_QSPI;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_QSPI;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    hqspi.Instance = QUADSPI;

    hqspi.Init.ClockPrescaler = 1;
    hqspi.Init.FifoThreshold = 1;
    hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
    hqspi.Init.FlashSize = 24;
    hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_5_CYCLE;
    hqspi.Init.ClockMode = QSPI_CLOCK_MODE_0;
    hqspi.Init.FlashID = QSPI_FLASH_ID_1;
    hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE;

    if (HAL_QSPI_Init(&hqspi) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    flash_wel();

    HAL_Delay(5);

    flash_wen();
    flash_set_qspi_mode();

    HAL_Delay(5);

    // Read ID
    QSPI_CommandTypeDef cmd = flash_get_default_cmd();
    cmd.Instruction = 0x9E;
    cmd.NbData = 3;
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    uint8_t id[3];
    HAL_QSPI_Command(&hqspi, &cmd, 100);
    HAL_QSPI_Receive(&hqspi, id, 100);

    printf("Mfr ID: 0x%02x, Dev ID: 0x%02x 0x%02x\n", id[0], id[1], id[2]);

    QSPI_MemoryMappedTypeDef cfg;
    cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;

    return STATUS_OK;
}