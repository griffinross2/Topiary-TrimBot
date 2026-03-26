#include "stm32h7xx_hal.h"
#include "clocks.h"
#include "board.h"
#include "gpio/gpio.h"
#include "terminal.h"
#include "flash.h"
#include "sdmmc/sdmmc.h"

#include <stdio.h>

int main(void)
{
    HAL_Init();

    int init_stat = STATUS_OK;
    init_stat |= clocks_init();
    init_stat |= terminal_init();
    init_stat |= flash_init();
    init_stat |= sdmmc_init(SD_SPEED_HIGH);

    if (init_stat != STATUS_OK) {
        gpio_write(PIN_RED, GPIO_HIGH);
    } else {
        gpio_write(PIN_GRN, GPIO_HIGH);
    }

    gpio_write(PIN_TURNTABLE_DIR, GPIO_HIGH);
    gpio_write(PIN_GANTRY_DIR, GPIO_HIGH);
    gpio_write(PIN_EXTRUDER_DIR, GPIO_HIGH);
    gpio_write(PIN_REVOLUTE_DIR, GPIO_HIGH);

    // Main loop
    while (1)
    {
        //gpio_write(PIN_PD3, GPIO_HIGH);
        //HAL_Delay(500);
        //gpio_write(PIN_PD3, GPIO_LOW);
        //HAL_Delay(500);

        gpio_write(PIN_TURNTABLE_STEP, GPIO_HIGH);
        gpio_write(PIN_GANTRY_STEP, GPIO_HIGH);
        gpio_write(PIN_EXTRUDER_STEP, GPIO_HIGH);
        gpio_write(PIN_REVOLUTE_STEP, GPIO_HIGH);
        HAL_Delay(15);
        gpio_write(PIN_TURNTABLE_STEP, GPIO_LOW);
        gpio_write(PIN_GANTRY_STEP, GPIO_LOW);
        gpio_write(PIN_EXTRUDER_STEP, GPIO_LOW);
        gpio_write(PIN_REVOLUTE_STEP, GPIO_LOW);
        HAL_Delay(15);
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