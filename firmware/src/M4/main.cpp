#include "stm32h7xx_hal.h"
#include "marlin_wrapper.h"
#include "gpio/gpio.h"
#include "board.h"

#include <stdio.h>

void main_loop();

int main(void)
{
    HAL_Init();

    // Marlin wrapper hosts the loop
    marlin_wrapper_set_idle_cb(main_loop);
    marlin_wrapper_loop();

    return 0;
}

void main_loop() {
    gpio_write(PIN_BLU, GPIO_HIGH);
    HAL_Delay(1000);
    gpio_write(PIN_BLU, GPIO_LOW);
    HAL_Delay(1000);
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
    void HSEM_IRQHandler(void);
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

void HSEM2_IRQHandler(void) {
    if (__HAL_HSEM_GET_FLAG(__HAL_HSEM_SEMID_TO_MASK(0))) {
        __HAL_HSEM_CLEAR_FLAG(__HAL_HSEM_SEMID_TO_MASK(0));
    }
}
