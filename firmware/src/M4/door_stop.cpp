#include "door_stop.h"

#include "board.h"
#include "gpio/gpio.h"
#include "module/planner.h"

#include "stm32h7xx_hal.h"

static EXTI_HandleTypeDef hexti;

Status door_stop_init() {
    gpio_mode(PIN_DOOR, GPIO_INPUT_PULLUP);

    EXTI_ConfigTypeDef exti_config = {};
    exti_config.Line = EXTI_LINE_3;
    exti_config.Mode = EXTI_MODE_INTERRUPT;
    exti_config.Trigger = EXTI_TRIGGER_RISING_FALLING;
    exti_config.GPIOSel = EXTI_GPIOB;

    if (HAL_EXTI_SetConfigLine(&hexti, &exti_config) != HAL_OK) {
        return STATUS_ERROR;
    }

    NVIC_SetPriority(EXTI3_IRQn, 0);
    NVIC_EnableIRQ(EXTI3_IRQn);

    return STATUS_OK;
}

void door_stop_task() {
    if (gpio_read(PIN_DOOR) == GPIO_HIGH) {
        // Door was opened - stop immediately
        planner.quick_stop();
        TRACE_PRINTF("Door opened, stopping motors...\n");
    } else {
        // Door was closed
        TRACE_PRINTF("Door closed.\n");
    }
}

extern "C" {
void EXTI3_IRQHandler();
}

void EXTI3_IRQHandler() {
    HAL_EXTI_IRQHandler(&hexti);

    door_stop_task();
}