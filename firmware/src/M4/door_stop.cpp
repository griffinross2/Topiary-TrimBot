#include "door_stop.h"

#include "board.h"
#include "gpio/gpio.h"
#include "module/planner.h"

#include "stm32h7xx_hal.h"

static EXTI_HandleTypeDef hexti;
static bool was_opened_last = false;

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
        // Stop the steppers
        planner.quick_stop();

        // Kill Marlin
        marlin_wrapper_kill();

        // Stop the cutter
        gpio_write(PIN_CUTTER, GPIO_LOW);

        if (!was_opened_last) {
            // Door was just opened
            TRACE_PRINTF("Door opened\n");
        }
        was_opened_last = true;
    } else {
        if (was_opened_last) {
            // Door was just closed
            TRACE_PRINTF("Door closed\n");
        }
        was_opened_last = false;
    }
}

extern "C" {
void EXTI3_IRQHandler();
}

void EXTI3_IRQHandler() {
    HAL_EXTI_IRQHandler(&hexti);

    door_stop_task();
}