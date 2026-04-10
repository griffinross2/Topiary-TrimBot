#include "stm32h7xx_hal.h"
#include "marlin_wrapper.h"
#include "gpio/gpio.h"
#include "board.h"
#include "gcode/gcode.h"
#include "terminal.h"

#include <stdio.h>

void main_loop();

int main(void)
{
    HAL_Init();

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
    gpio_write(PIN_BLU, GPIO_HIGH);
    HAL_Delay(1000);
    gpio_write(PIN_BLU, GPIO_LOW);
    HAL_Delay(1000);

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