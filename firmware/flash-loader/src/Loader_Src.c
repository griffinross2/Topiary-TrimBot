#include "Loader_Src.h"

#include "flash.h"
#include "stm32h7xx_hal.h"
#include "clocks.h"
#include "gpio.h"
#include "board.h"
#include "terminal.h"

int main()
{
    return 0;
}

// From https://github.com/STMicroelectronics/stm32-memory-loaders/blob/main/STM32H5x_boards/MX25LM51245G_STM32H573I-DK/Flash_loader/Src/Loader_Src.c
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
    /* Declare and initialize retr */
    HAL_StatusTypeDef retr = HAL_ERROR;

    /* Check uwTickFreq for MisraC 2012 (even if uwTickFreq is a enum type that doesn't take the value zero) */
    if ((uint32_t)uwTickFreq != 0U)
    {
        uint32_t ticks = SystemCoreClock / (1000U / (uint32_t)uwTickFreq);
        SysTick->LOAD = (uint32_t)(ticks - 1UL);     /* Set reload register */
        SysTick->VAL = 0UL;                          /* Load the SysTick Counter Value */
        SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | /* Set processor clock */
                        SysTick_CTRL_ENABLE_Msk;     /* Enable SysTick Timer */
        retr = HAL_OK;
    }

    /* Return status of the HAL operation */
    return retr;
}

// From https://github.com/STMicroelectronics/stm32-memory-loaders/blob/main/STM32H5x_boards/MX25LM51245G_STM32H573I-DK/Flash_loader/Src/Loader_Src.c
/**
  * @brief Provide a tick value in millisecond.
  * @note The function is an override of the HAL function to increment the
  *       tick on a count flag event.
  * @retval tick value
  */
uint32_t HAL_GetTick(void)
{
    /* Check if the SysTick counter flag is set */
    if ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == SysTick_CTRL_COUNTFLAG_Msk)
    {
        /* Increment the tick counter */
        uwTick++;
    }

    /* Return the current tick value */
    return uwTick;
}

__attribute__((used)) int Init(void) {
    SystemInit();

    SCB->VTOR = 0x24000000 | 0x200;

    HAL_DeInit();
    HAL_Init();

    if (clocks_init() != STATUS_OK)
    {
        return 0;
    }

    gpio_write(PIN_BLU, GPIO_HIGH);

    if (terminal_init() != STATUS_OK)
    {
        return 0;
    }

    TRACE_PRINTF("Terminal initialized\n");

    if (flash_init() != STATUS_OK)
    {
        TRACE_PRINTF("Flash init failed\n");
        return 0;
    }

    TRACE_PRINTF("Flash initialized\n");

    if (flash_set_memory_mapped_mode() != STATUS_OK)
    {
        TRACE_PRINTF("Flash memory mapped mode failed\n");
        return 0;
    }

    TRACE_PRINTF("Flash memory mapped mode set\n");

    return 1;
}

__attribute__((used)) int Write(uint32_t Address, uint32_t Size, uint8_t* buffer) {
    // 256 byte pages

    if (flash_clear_memory_mapped_mode() != STATUS_OK)
    {
        TRACE_PRINTF("Clearing flash memory mapped mode failed\n");
        return 0;
    }

    while (Size > 0)
    {
        int remaining_in_page = 256 - (Address % 256);
        int chunk_size = Size < remaining_in_page ? Size : remaining_in_page;

        if (flash_write(Address - 0x90000000, buffer, chunk_size) != STATUS_OK)
        {
            TRACE_PRINTF("Flash write failed at address 0x%08X\n", Address);
            return 0;
        }

        Address += chunk_size;
        buffer += chunk_size;
        Size -= chunk_size;
    }

    return 1;
}

__attribute__((used)) int SectorErase(uint32_t StartAddress, uint32_t EndAddress) {
    // 64KB sectors

    if (flash_clear_memory_mapped_mode() != STATUS_OK)
    {
        TRACE_PRINTF("Clearing flash memory mapped mode failed\n");
        return 0;
    }

    for (uint32_t sect = (StartAddress - 0x90000000) >> 16; sect <= (EndAddress - 0x90000000) >> 16; sect += 1)
    {
        TRACE_PRINTF("Erasing flash sector 0x%08X\n", sect);

        if (flash_erase_sector(sect << 16) != STATUS_OK)
        {
            TRACE_PRINTF("Flash sector erase failed at sector 0x%08X\n", sect);
            return 0;
        }
    }

    return 1;
}

__attribute__((used)) uint64_t Verify(uint32_t FlashAddr, uint32_t RAMBufferAddr, uint32_t Size, uint32_t misalignment) {
    uint32_t VerifiedData = 0;
    uint32_t InitVal = 0;
    uint64_t checksum;
    Size *= 4;
    
    if (flash_set_memory_mapped_mode() != STATUS_OK)
    {
        TRACE_PRINTF("Flash memory mapped mode failed\n");
        return 0;
    }

    /* Calculate checksum of the memory region */
    checksum = CheckSum((uint32_t)FlashAddr + (misalignment & 0xf), Size - ((misalignment >> 16) & 0xF), InitVal);

    /* Verify the memory region against the RAM buffer */
    while (Size > VerifiedData)
    {
        if (*(uint8_t *)FlashAddr++ != *((uint8_t *)RAMBufferAddr + VerifiedData))
        {
            /* Return the address of failure and checksum */
            return ((checksum << 32) + (FlashAddr + VerifiedData));
        }

        VerifiedData++;
    }

    /* Return the checksum value */
    return (checksum << 32);
}

__attribute__((used)) int MassErase(uint32_t Parallelism) {
    if (flash_clear_memory_mapped_mode() != STATUS_OK)
    {
        TRACE_PRINTF("Clearing flash memory mapped mode failed\n");
        return 0;
    }

    if (flash_erase_chip() != STATUS_OK)
    {
        TRACE_PRINTF("Flash chip erase failed\n");
        return 0;
    }

    return 1;
}

__attribute__((used)) uint32_t CheckSum(uint32_t StartAddress, uint32_t Size, uint32_t InitVal) {
    /* Calculate misalignment of the start address */
    uint8_t misalignment_address = StartAddress % 4;
    /* Store the original size for later use */
    uint8_t misalignment_size = Size;
    uint32_t cnt;
    uint32_t Val;

    /* Align the start address to the nearest 4-byte boundary */
    StartAddress -= StartAddress % 4;
    /* Adjust the size to be a multiple of 4 bytes */
    Size += (Size % 4 == 0) ? 0 : 4 - (Size % 4);

    /* Iterate over the memory region in 4-byte chunks */
    for (cnt = 0; cnt < Size; cnt += 4)
    {
        /* Read a 4-byte value from the current address */
        Val = *(uint32_t *)StartAddress;

        /* Handle initial misalignment of the start address */
        if (misalignment_address)
        {
            switch (misalignment_address)
            {
            case 1:
                InitVal += (uint8_t)(Val >> 8 & 0xff);
                InitVal += (uint8_t)(Val >> 16 & 0xff);
                InitVal += (uint8_t)(Val >> 24 & 0xff);
                misalignment_address -= 1;
                break;
            case 2:
                InitVal += (uint8_t)(Val >> 16 & 0xff);
                InitVal += (uint8_t)(Val >> 24 & 0xff);
                misalignment_address -= 2;
                break;
            case 3:
                InitVal += (uint8_t)(Val >> 24 & 0xff);
                misalignment_address -= 3;
                break;
            }
        }
        /* Handle final misalignment of the size */
        else if ((Size - misalignment_size) % 4 && (Size - cnt) <= 4)
        {
            switch (Size - misalignment_size)
            {
            case 1:
                InitVal += (uint8_t)Val;
                InitVal += (uint8_t)(Val >> 8 & 0xff);
                InitVal += (uint8_t)(Val >> 16 & 0xff);
                misalignment_size -= 1;
                break;
            case 2:
                InitVal += (uint8_t)Val;
                InitVal += (uint8_t)(Val >> 8 & 0xff);
                misalignment_size -= 2;
                break;
            case 3:
                InitVal += (uint8_t)Val;
                misalignment_size -= 3;
                break;
            }
        }
        /* Process aligned 4-byte chunks */
        else
        {
            InitVal += (uint8_t)Val;
            InitVal += (uint8_t)(Val >> 8 & 0xff);
            InitVal += (uint8_t)(Val >> 16 & 0xff);
            InitVal += (uint8_t)(Val >> 24 & 0xff);
        }
        /* Move to the next 4-byte chunk */
        StartAddress += 4;
    }

    /* Return the calculated checksum value */
    return InitVal;
}

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

void NMI_Handler(void)
{
    gpio_write(PIN_RED, GPIO_HIGH);
    while (1)
    {
    }
}

void HardFault_Handler(void)
{
    gpio_write(PIN_RED, GPIO_HIGH);
    while (1)
    {
    }
}

void MemManage_Handler(void)
{
    gpio_write(PIN_RED, GPIO_HIGH);
    while (1)
    {
    }
}

void BusFault_Handler(void)
{
    gpio_write(PIN_RED, GPIO_HIGH);
    while (1)
    {
    }
}

void UsageFault_Handler(void)
{
    gpio_write(PIN_RED, GPIO_HIGH);
    while (1)
    {
    }
}

void SVC_Handler(void) {}

void DebugMon_Handler(void) {}

void PendSV_Handler(void) {}

void SysTick_Handler(void)
{
    HAL_IncTick();
}