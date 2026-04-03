#ifndef MARLIN_WRAPPER_H
#define MARLIN_WRAPPER_H

#include <stddef.h>
#include <stdint.h>
// #include "macros.h"
#include "stm32h7xx_hal.h"

void marlin_wrapper_init();
void marlin_wrapper_set_idle_cb(void (*cb)(void));
void marlin_wrapper_loop();
void marlin_wrapper_idle();
size_t marlin_wrapper_serial_available();
int marlin_wrapper_serial_read();
void marlin_wrapper_kill();
bool marlin_wrapper_is_running();
bool marlin_wrapper_is_printer_busy();
uint32_t marlin_wrapper_step_timer_count();

enum MarlinState : uint8_t {
    MF_INITIALIZING = 0,
    MF_STOPPED,
    MF_KILLED,
    MF_RUNNING,
    MF_SD_COMPLETE,
    MF_PAUSED,
    MF_WAITING,
};
/*
#define MF_TIMER_STEP 0  // Timer Index for Stepper
#define MF_TIMER_PULSE MF_TIMER_STEP
#define STEP_TIMER_IRQ_PRIO      2

#define TIMER_INDEX_(T) \
    TIMER##T##_INDEX  // TIMER#_INDEX enums (timer_index_t) depend on TIM#_BASE
                      // defines.
#define TIMER_INDEX(T) \
    TIMER_INDEX_(T)  // Convert Timer ID to HardwareTimer_Handle index.

#ifndef HAL_TIMER_RATE
extern uint32_t GetStepperTimerClkFreq();
#define HAL_TIMER_RATE GetStepperTimerClkFreq()
#endif

#define HAL_TIMER_TYPE_MAX uint32_t(UINT32_MAX)
#define STEPPER_TIMER_RATE 2000000
// Timer prescaler calculations
#define STEPPER_TIMER_PRESCALE ((HAL_TIMER_RATE) / (STEPPER_TIMER_RATE))
#define STEPPER_TIMER_TICKS_PER_US \
    ((STEPPER_TIMER_RATE) / 1000000UL)  // (ticks/μs) Stepper Timer ticks per µs

#define ENABLE_STEPPER_DRIVER_INTERRUPT() \
    HAL_timer_enable_interrupt(MF_TIMER_STEP)
#define DISABLE_STEPPER_DRIVER_INTERRUPT() \
    HAL_timer_disable_interrupt(MF_TIMER_STEP)
#define STEPPER_ISR_ENABLED() HAL_timer_interrupt_enabled(MF_TIMER_STEP)

#define ENABLE_TEMPERATURE_INTERRUPT() HAL_timer_enable_interrupt(MF_TIMER_TEMP)
#define DISABLE_TEMPERATURE_INTERRUPT() \
    HAL_timer_disable_interrupt(MF_TIMER_TEMP)

extern void Step_Handler();
extern void Temp_Handler();

#ifndef HAL_STEP_TIMER_ISR
#define HAL_STEP_TIMER_ISR() void Step_Handler()
#endif
#ifndef HAL_TEMP_TIMER_ISR
#define HAL_TEMP_TIMER_ISR() void Temp_Handler()
#endif

extern TIM_HandleTypeDef timer_instance[];

void HAL_timer_start(const uint8_t timer_num, const uint32_t frequency);
void HAL_timer_enable_interrupt(const uint8_t timer_num);
void HAL_timer_disable_interrupt(const uint8_t timer_num);
bool HAL_timer_interrupt_enabled(const uint8_t timer_num);

// Configure timer priorities for peripherals such as Software Serial or Servos.
// Exposed here to allow all timer priority information to reside in timers.cpp
void SetTimerInterruptPriorities();

// FORCE_INLINE because these are used in performance-critical situations
FORCE_INLINE bool HAL_timer_initialized(const uint8_t timer_num) {
    return timer_instance[timer_num].Instance != nullptr;
}
FORCE_INLINE static uint32_t HAL_timer_get_count(const uint8_t timer_num) {
    return HAL_timer_initialized(timer_num)
               ? __HAL_TIM_GetCounter(&timer_instance[timer_num])
               : 0;
}

// NOTE: Method name may be misleading.
// STM32 has an Auto-Reload Register (ARR) as opposed to a "compare" register
FORCE_INLINE static void HAL_timer_set_compare(const uint8_t timer_num,
                                               const uint32_t overflow) {
    if (HAL_timer_initialized(timer_num)) {
        timer_instance[timer_num].Instance->ARR = overflow;
        // Value decremented by setOverflow()
        // wiki: "force all registers (Autoreload, prescaler, compare) to be
        // taken into account" So, if the new overflow value is less than the
        // count it will trigger a rollover interrupt.
        if (overflow < __HAL_TIM_GetCounter(&timer_instance[timer_num]))  // Added 'if' here because reports say
                                          // it won't boot without it
            HAL_TIM_GenerateEvent(&timer_instance[timer_num], TIM_EVENTSOURCE_UPDATE);
    }
}

inline void HAL_timer_isr_prologue(const uint8_t) {}
inline void HAL_timer_isr_epilogue(const uint8_t) {}
*/

#endif  // MARLIN_WRAPPER_H