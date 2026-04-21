#include "timing.h"

#include "stm32h7xx_hal.h"

void delay_ms(unsigned int ms) {
    HAL_Delay(ms);
}

uint32_t get_tick_ms() {
    return HAL_GetTick();
}