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

#include <stdio.h>

int main(void)
{
    HAL_Init();

    int init_stat = STATUS_OK;
    init_stat |= clocks_init();
    init_stat |= terminal_init() << 1;
    init_stat |= flash_init() << 2;
    init_stat |= sdmmc_init(SD_SPEED_HIGH) << 3;
    init_stat |= tsc2013_init() << 4;
    init_stat |= lcd_init() << 5;    

    // lcd_draw_rectangle(0, 0, LTDC_WINDOW_WIDTH, LTDC_WINDOW_HEIGHT, 0xF2);
    // lcd_draw_text(&ARIAL, "Hello, World!", 50, 160, 48, 0xF1);
    lcd_get_backbuffer()[0] = 0xF2;
    lcd_get_backbuffer()[1] = 0xF2;
    lcd_get_backbuffer()[2] = 0xF2;
    lcd_get_backbuffer()[3] = 0xF2;
    lcd_get_backbuffer()[4] = 0xF2;
    lcd_swap_buffers();

    HAL_Delay(1000);

    printf("Init status: 0x%x\n", init_stat);

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

    // Main loop
    while (1)
    {
        // gpio_write(PIN_PD3, GPIO_HIGH);
        // HAL_Delay(500);
        // gpio_write(PIN_PD3, GPIO_LOW);
        // HAL_Delay(500);
        if (gpio_read(PIN_BL_DISC) == GPIO_LOW)
        {
            gpio_write(PIN_YEL, GPIO_HIGH);
        }
        else
        {
            gpio_write(PIN_YEL, GPIO_LOW);
        }

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