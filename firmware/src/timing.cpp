#include "timing.h"

#include "stm32h7xx_hal.h"

void delay_ms(unsigned int ms)
{
    HAL_Delay(ms);
}

long long unsigned get_tick_ms()
{
    return (long long unsigned)HAL_GetTick();
}