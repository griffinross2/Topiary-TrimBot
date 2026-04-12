#include "stm32h7xx_hal.h"
#include "marlin_wrapper.h"
#include "gpio/gpio.h"
#include "board.h"
#include "gcode/gcode.h"
#include "terminal.h"
#include "profiler.h"

#include <stdio.h>

void main_loop();

int main(void)
{
    HAL_Init();

    profiler_init();

    terminal_rx_start();

    marlin_wrapper_init();

    // Marlin wrapper hosts the loop
    marlin_wrapper_set_idle_cb(main_loop);

    // gcode.home_all_axes();
    
    marlin_wrapper_loop();

    return 0;
}

char buf[64];

void main_loop() {
    static uint32_t profiler_tick = HAL_GetTick();
    static uint32_t blinky_tick = HAL_GetTick();

    // Blink blue LED for status
    if (get_tick_ms() - blinky_tick >= 1000) {
        gpio_toggle(PIN_BLU);
        blinky_tick = get_tick_ms();
    }

    // Print profiler summary every 10 seconds
    if (get_tick_ms() - profiler_tick >= 10000) {
        profiler_print_summary();
        profiler_tick = get_tick_ms();
    }

    // Process manual g-code input
    int bytes_read = terminal_read((uint8_t*)buf, sizeof(buf)-1);
    if (bytes_read > 0) {
        buf[bytes_read] = '\0';
        printf("%s", buf);
        gcode.process_subcommands_now(buf);
    }
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