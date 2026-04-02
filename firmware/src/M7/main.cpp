#include "stm32h7xx_hal.h"
#include "clocks.h"
#include "board.h"
#include "gpio/gpio.h"
#include "terminal.h"
#include "flash.h"
#include "sdmmc/sdmmc.h"
#include "tsc2013/tsc2013.h"
#include "ltdc.h"
#include "lcd.h"
#include "fonts/arial.h"
#include "images/blank.h"
#include "gui_app.h"
#include "filesystem.h"

#include <stdio.h>

int main(void)
{
    HAL_Init();

    DBGMCU->CR |= DBGMCU_CR_DBG_CKD1EN | DBGMCU_CR_DBG_CKD3EN;

    int init_stat = STATUS_OK;
    init_stat |= clocks_init();
    init_stat |= terminal_init() << 1;
    init_stat |= flash_init() << 2;
    init_stat |= filesystem_init() << 3;
    init_stat |= tsc2013_init() << 4;
    init_stat |= lcd_init() << 5;
    init_stat |= gui_app_init() << 6;

    printf("Init status: 0x%x\n", init_stat);
    HAL_RCCEx_EnableBootCore(RCC_BOOT_C2);

    if (init_stat != STATUS_OK)
    {
        gpio_write(PIN_RED, GPIO_HIGH);
    }
    else
    {
        gpio_write(PIN_GRN, GPIO_HIGH);
    }

    gpio_write(PIN_TURNTABLE_DIR, GPIO_HIGH);
    gpio_write(PIN_GANTRY_DIR, GPIO_HIGH);
    gpio_write(PIN_EXTRUDER_DIR, GPIO_HIGH);
    gpio_write(PIN_REVOLUTE_DIR, GPIO_HIGH);

    gpio_write(PIN_CUTTER, GPIO_HIGH);

    std::vector<FileInfo> file_list;
    filesystem_get_file_list(file_list);
    for (size_t i = 0; i < file_list.size(); i++)
    {
        printf("File %lu: %s (%lu bytes)\n", (uint32_t)i, file_list[i].name.c_str(), (uint32_t)file_list[i].size);
    }

    // Main loop
    while (1)
    {
        // gpio_write(PIN_BLU, GPIO_HIGH);
        // HAL_Delay(500);
        // gpio_write(PIN_BLU, GPIO_LOW);
        // HAL_Delay(500);
        // if (gpio_read(PIN_BL_DISC) == GPIO_LOW)
        // {
        //     gpio_write(PIN_YEL, GPIO_HIGH);
        // }
        // else
        // {
        //     gpio_write(PIN_YEL, GPIO_LOW);
        // }

        filesystem_task();
        gui_app_task();
    }

    return 0;
}

extern "C"
{
    void NMI_Handler(void);
    void HardFault_Handler(void);
    void MemManage_Handler(void);
    void BusFault_Handler(void);
    void UsageFault_Handler(void);
    void SVC_Handler(void);
    void DebugMon_Handler(void);
    void PendSV_Handler(void);
    void SysTick_Handler(void);
}

void NMI_Handler(void)
{
    while (1)
    {
    }
}

void HardFault_Handler(void)
{
    while (1)
    {
    }
}

void MemManage_Handler(void)
{
    while (1)
    {
    }
}

void BusFault_Handler(void)
{
    while (1)
    {
    }
}

void UsageFault_Handler(void)
{
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