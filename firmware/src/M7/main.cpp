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
#include "images/calibration.h"
#include "images/splashscreen.h"

#include <stdio.h>

int tx = 0, ty = 0;

void tsc2013_touch_callback(int x, int y) {
    tx = x;
    ty = y;
    printf("Touch callback - X: %d, Y: %d\n", x, y);
}

int main(void)
{
    HAL_Init();

    DBGMCU->CR |= DBGMCU_CR_DBG_CKD1EN | DBGMCU_CR_DBG_CKD3EN;

    int init_stat = STATUS_OK;
    init_stat |= clocks_init();
    init_stat |= terminal_init() << 1;
    init_stat |= flash_init() << 2;
    init_stat |= sdmmc_init(SD_SPEED_HIGH) << 3;
    init_stat |= tsc2013_init() << 4;
    init_stat |= lcd_init() << 5;

    tsc2013_set_touch_callback(tsc2013_touch_callback);

    // lcd_draw_text(&ARIAL, "Hello, World!", 240, 150, 48, 0xF1);
    // lcd_swap_buffers();
    lcd_set_background(CALIBRATION);
    // lcd_set_background(SPLASHSCREEN);

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

    // Main loop
    uint32_t last_tick = 0;
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

        // int tx_window = tx - (LCD_WIDTH - WINDOW_WIDTH) / 2;
        // int ty_window = ty - (LCD_HEIGHT - WINDOW_HEIGHT) / 2;
        // if (tx_window >= 0 && tx_window < WINDOW_WIDTH && ty_window >= 0 && ty_window < WINDOW_HEIGHT)
        // {
        //     lcd_clear_foreground();
        //     lcd_draw_circle(tx_window, ty_window, 10, 0xF2);
        //     lcd_swap_buffers();
        // }

        gpio_write(PIN_GANTRY_STEP, GPIO_HIGH);
        HAL_Delay(20);
        gpio_write(PIN_GANTRY_STEP, GPIO_LOW);
        HAL_Delay(20);

        if (HAL_GetTick() - last_tick > 5000)
        {
            gpio_write(PIN_GANTRY_DIR, GPIO_HIGH);
        }
        if (HAL_GetTick() - last_tick > 10000)
        {
            gpio_write(PIN_GANTRY_DIR, GPIO_LOW);
            last_tick = HAL_GetTick();
        }
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