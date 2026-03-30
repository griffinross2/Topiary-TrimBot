#include "flash.h"

#include "stm32h7xx_hal.h"

#include <stdio.h>
#include <string.h>

QSPI_HandleTypeDef hqspi;

static const QSPI_CommandTypeDef s_qspi_default_cmd_1_line = {
    .Instruction = 0,
    .Address = 0,
    .AlternateBytes = 0,
    .AddressSize = QSPI_ADDRESS_24_BITS,
    .AlternateBytesSize = 0,
    .DummyCycles = 0,
    .InstructionMode = QSPI_INSTRUCTION_1_LINE,
    .AddressMode = QSPI_ADDRESS_1_LINE,
    .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
    .DataMode = QSPI_DATA_1_LINE,
    .NbData = 0,
    .DdrMode = QSPI_DDR_MODE_DISABLE,
    .DdrHoldHalfCycle = 0,
    .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,
};

static const QSPI_CommandTypeDef s_qspi_default_cmd_4_lines = {
    .Instruction = 0,
    .Address = 0,
    .AlternateBytes = 0,
    .AddressSize = QSPI_ADDRESS_24_BITS,
    .AlternateBytesSize = 0,
    .DummyCycles = 0,
    .InstructionMode = QSPI_INSTRUCTION_4_LINES,
    .AddressMode = QSPI_ADDRESS_4_LINES,
    .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
    .DataMode = QSPI_DATA_4_LINES,
    .NbData = 0,
    .DdrMode = QSPI_DDR_MODE_DISABLE,
    .DdrHoldHalfCycle = 0,
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
        return s_qspi_default_cmd_4_lines;
        break;
    }

    return s_qspi_default_cmd_1_line;
}

static Status flash_read_status(uint8_t *status)
{
    QSPI_CommandTypeDef cmd = flash_get_default_cmd();
    cmd.Instruction = 0x05;
    cmd.NbData = 1;
    cmd.AddressMode = QSPI_ADDRESS_NONE;

    if (HAL_QSPI_Command(&hqspi, &cmd, 100) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    if (HAL_QSPI_Receive(&hqspi, status, 100) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

static Status flash_read_config(uint8_t *config)
{
    QSPI_CommandTypeDef cmd = flash_get_default_cmd();
    cmd.Instruction = 0x15;
    cmd.NbData = 1;
    cmd.AddressMode = QSPI_ADDRESS_NONE;

    if (HAL_QSPI_Command(&hqspi, &cmd, 100) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    if (HAL_QSPI_Receive(&hqspi, config, 100) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

static Status flash_wait_ready(uint32_t timeout)
{
    uint32_t tickstart = HAL_GetTick();
    uint8_t status = 0x1;
    while (status & 0x1)
    {
        if (HAL_GetTick() - tickstart >= timeout)
        {
            return STATUS_TIMEOUT;
        }

        flash_read_status(&status);
    }

    return STATUS_OK;
}

static Status flash_wen(uint32_t timeout)
{
    uint8_t sr = 0;

    QSPI_CommandTypeDef wen_cmd = flash_get_default_cmd();
    wen_cmd.Instruction = 0x06;
    wen_cmd.NbData = 0;
    wen_cmd.AddressMode = QSPI_ADDRESS_NONE;
    wen_cmd.DataMode = QSPI_DATA_NONE;

    while ((sr & 0x2) == 0) {
        // Write enable
        if (HAL_QSPI_Command(&hqspi, &wen_cmd, 100) != HAL_OK)
        {
            return STATUS_ERROR;
        }

        // Read status
        if (flash_read_status(&sr) != STATUS_OK)
        {
            return STATUS_ERROR;
        }
    }

    return STATUS_OK;
}

static Status flash_write_status_config(uint16_t status_config)
{
    if (flash_wen(100) != STATUS_OK)
    {
        return STATUS_ERROR;
    }

    QSPI_CommandTypeDef cmd = flash_get_default_cmd();
    cmd.Instruction = 0x01;
    cmd.NbData = 2;
    cmd.AddressMode = QSPI_ADDRESS_NONE;

    if (HAL_QSPI_Command(&hqspi, &cmd, 100) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    if (HAL_QSPI_Transmit(&hqspi, (uint8_t*)&status_config, 100) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    flash_wait_ready(100);

    return STATUS_OK;
}

static Status flash_set_qspi_mode()
{
    uint16_t status_config = 0;
    if (flash_read_status((uint8_t*)&status_config) != STATUS_OK)
    {
        return STATUS_ERROR;
    }

    if (flash_read_config((uint8_t*)&status_config + 1) != STATUS_OK)
    {
        return STATUS_ERROR;
    }

    // Set QE bit
    status_config |= 0x40;

    if (flash_write_status_config(status_config) != STATUS_OK)
    {
        return STATUS_ERROR;
    }

    QSPI_CommandTypeDef cmd = flash_get_default_cmd();
    cmd.Instruction = 0x35;
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.DataMode = QSPI_DATA_NONE;
    if (HAL_QSPI_Command(&hqspi, &cmd, 100) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    qspi_mode = 1;

    return STATUS_OK;
}

static Status flash_clear_qspi_mode()
{
    QSPI_CommandTypeDef cmd = s_qspi_default_cmd_4_lines;
    cmd.Instruction = 0xF5;
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.DataMode = QSPI_DATA_NONE;
    if (HAL_QSPI_Command(&hqspi, &cmd, 100) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    qspi_mode = 0;

    return STATUS_OK;
}

static Status flash_reset()
{
    // First in QSPI mode
    QSPI_CommandTypeDef cmd = s_qspi_default_cmd_4_lines;
    cmd.Instruction = 0x66; // Reset Enable
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.DataMode = QSPI_DATA_NONE;

    if (HAL_QSPI_Command(&hqspi, &cmd, 100) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    cmd.Instruction = 0x99; // Reset Memory

    if (HAL_QSPI_Command(&hqspi, &cmd, 100) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    HAL_Delay(100);

    // Then in SPI mode
    cmd = s_qspi_default_cmd_1_line;
    cmd.Instruction = 0x66; // Reset Enable
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.DataMode = QSPI_DATA_NONE;

    if (HAL_QSPI_Command(&hqspi, &cmd, 100) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    cmd.Instruction = 0x99; // Reset Memory

    if (HAL_QSPI_Command(&hqspi, &cmd, 100) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    HAL_Delay(100);

    return STATUS_OK;
}

static Status flash_check_id()
{
    QSPI_CommandTypeDef cmd = flash_get_default_cmd();
    cmd.Instruction = 0x9F;
    cmd.NbData = 3;
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    uint8_t id[3];
    if (HAL_QSPI_Command(&hqspi, &cmd, 100) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    if (HAL_QSPI_Receive(&hqspi, id, 100) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    printf("Flash ID: 0x%02x, 0x%02x, 0x%02x\n", id[0], id[1], id[2]);

    if (id[0] != 0xC2 || id[1] != 0x20 || id[2] != 0x1A)
    {
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

static QSPI_CommandTypeDef flash_get_mm_read_cmd()
{
    QSPI_CommandTypeDef cmd = flash_get_default_cmd();
    cmd.Instruction = 0xEB;
    cmd.AddressMode = QSPI_ADDRESS_4_LINES;
    cmd.DataMode = QSPI_DATA_4_LINES;
    cmd.DummyCycles = 10;

    return cmd;
}

Status flash_read(uint32_t addr, uint8_t *buf, int len)
{
    QSPI_CommandTypeDef cmd = flash_get_default_cmd();
    cmd.Instruction = 0xEB;
    cmd.AddressMode = QSPI_ADDRESS_4_LINES;
    cmd.DataMode = QSPI_DATA_4_LINES;
    cmd.DummyCycles = 10;
    cmd.NbData = len;
    cmd.Address = addr;

    if (HAL_QSPI_Command(&hqspi, &cmd, 100) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    if (HAL_QSPI_Receive(&hqspi, buf, 100) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

static Status flash_set_dummy_cycles(uint8_t dummy_cycles)
{
    uint16_t status_config = 0;

    if (flash_read_status((uint8_t*)&status_config) != STATUS_OK)
    {
        return STATUS_ERROR;
    }

    if (flash_read_config((uint8_t*)&status_config + 1) != STATUS_OK)
    {
        return STATUS_ERROR;
    }

    status_config &= ~(0xC000);
    status_config |= dummy_cycles << 14;

    if (flash_write_status_config(status_config) != STATUS_OK)
    {
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

Status flash_init()
{
    qspi_mode = 0;

    __HAL_RCC_QSPI_CLK_ENABLE();
    __HAL_RCC_QSPI_FORCE_RESET();
    __HAL_RCC_QSPI_RELEASE_RESET();

    GPIO_InitTypeDef GPIO_InitStruct;

    // CS
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    // D2-3 and CLK
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    hqspi.Instance = QUADSPI;

    hqspi.Init.ClockPrescaler = 1; // 240 MHz / 2 = 120 MHz
    hqspi.Init.FifoThreshold = 1;
    hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
    hqspi.Init.FlashSize = 25;
    hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_5_CYCLE;
    hqspi.Init.ClockMode = QSPI_CLOCK_MODE_0;
    hqspi.Init.FlashID = QSPI_FLASH_ID_1;
    hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE;

    if (HAL_QSPI_Init(&hqspi) != HAL_OK)
    {
        TRACE_PRINTF("QSPI init failed\n");
        return STATUS_ERROR;
    }

    // Reset
    if (flash_reset() != STATUS_OK)
    {
        TRACE_PRINTF("Flash reset failed\n");
        return STATUS_ERROR;
    }

    // Wait til ready
    flash_wait_ready(100);
    TRACE_PRINTF("Flash reset complete\n");

    // Verify hardware IDs
    if (flash_check_id() != STATUS_OK)
    {
        TRACE_PRINTF("Flash ID check failed\n");
        return STATUS_ERROR;
    }

    // Set dummy cycles for 120 MHz
    if (flash_set_dummy_cycles(0x3) != STATUS_OK)
    {
        TRACE_PRINTF("Setting dummy cycles failed\n");
        return STATUS_ERROR;
    }

    // Set QSPI mode
    if (flash_set_qspi_mode() != STATUS_OK)
    {
        TRACE_PRINTF("Setting QSPI mode failed\n");
        return STATUS_ERROR;
    }

    // Enter memory mapped mode
    if (flash_set_memory_mapped_mode() != STATUS_OK)
    {
        TRACE_PRINTF("Setting memory mapped mode failed\n");
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

Status flash_set_memory_mapped_mode() {
    // Set read command
    QSPI_MemoryMappedTypeDef cfg;
    cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
    cfg.TimeOutPeriod = 0;

    QSPI_CommandTypeDef cmd = flash_get_mm_read_cmd();
    if (HAL_QSPI_MemoryMapped(&hqspi, &cmd, &cfg) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

Status flash_clear_memory_mapped_mode() {
    if (HAL_QSPI_Abort(&hqspi) != HAL_OK)
    {        
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

Status flash_write(uint32_t addr, uint8_t *buf, int len) {
    QSPI_CommandTypeDef cmd = flash_get_default_cmd();
    cmd.Instruction = 0x02;
    cmd.NbData = len;
    cmd.Address = addr;

    if (flash_wen(100) != STATUS_OK)
    {
        return STATUS_ERROR;
    }

    if (HAL_QSPI_Command(&hqspi, &cmd, 100) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    if (HAL_QSPI_Transmit(&hqspi, buf, 100) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    flash_wait_ready(100);

    return STATUS_OK;
}

Status flash_erase_sector(uint32_t addr) {
    // Erase 64KB sector

    if (flash_wen(100) != STATUS_OK)
    {
        return STATUS_ERROR;
    }

    QSPI_CommandTypeDef cmd = flash_get_default_cmd();
    cmd.Instruction = 0xD8;
    cmd.Address = addr;
    cmd.DataMode = QSPI_DATA_NONE;

    if (HAL_QSPI_Command(&hqspi, &cmd, 100) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    flash_wait_ready(100);

    return STATUS_OK;
}

Status flash_erase_chip() {
    if (flash_wen(100) != STATUS_OK)
    {
        return STATUS_ERROR;
    }

    QSPI_CommandTypeDef cmd = flash_get_default_cmd();
    cmd.Instruction = 0x60;
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.DataMode = QSPI_DATA_NONE;

    if (HAL_QSPI_Command(&hqspi, &cmd, 100) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    flash_wait_ready(100);

    return STATUS_OK;
}