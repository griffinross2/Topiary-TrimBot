#include "marlin_wrapper.h"

// #include "gcode/queue.h"
// #include "module/endstops.h"
// #include "module/stepper.h"
#include "gpio/gpio.h"
// #include "Delay.h"
#include "memory.h"

void (*marlin_idle_cb)(void) = nullptr;

MarlinState marlin_state = MF_INITIALIZING;

void marlin_wrapper_init() {
  // Some HAL need precise delay adjustment
  // calibrate_delay_loop();

  // TERN_(HAS_HOME_OFFSET,
  //         current_position +=
  //         home_offset);  // Init current position based on home_offset

  // sync_plan_position();  // Vital to init stepper/planner equivalent for
  //                         // current_position

  // endstops.init();
  // stepper.init();
  marlin_state = MF_RUNNING;
}

void marlin_wrapper_set_idle_cb(void (*cb)(void)) {
    marlin_idle_cb = cb;
}

void marlin_wrapper_loop() {
    while(1) {
        // Other core tasks
        marlin_wrapper_idle();

        // queue.advance();
        // endstops.event_handler();
    }
}

void marlin_wrapper_idle() {
    if (marlin_idle_cb) {
        marlin_idle_cb();
    }
}

size_t marlin_wrapper_serial_available() {
    return 0;
}

int marlin_wrapper_serial_read() {
    return -1;
}

void marlin_wrapper_kill() {
    marlin_state = MF_KILLED;
}

bool marlin_wrapper_is_running() {
    return marlin_state >= MF_RUNNING;
}

bool marlin_wrapper_printer_busy() {
    // return planner.has_blocks_queued();
    return false;
}

uint32_t marlin_wrapper_step_timer_count() {
    return 0;
}


// #if defined(STM32F0xx) || defined(STM32G0xx)
//   #define MCU_STEP_TIMER 16
//   #define MCU_TEMP_TIMER 17
// #elif defined(STM32F1xx)
//   #define MCU_STEP_TIMER  4
//   #define MCU_TEMP_TIMER  2
// #elif defined(STM32F401xC) || defined(STM32F401xE)
//   #define MCU_STEP_TIMER  9           // STM32F401 has no TIM6, TIM7, or TIM8
//   #define MCU_TEMP_TIMER 10
// #elif defined(STM32F4xx) || defined(STM32F7xx) || defined(STM32H7xx)
//   #define MCU_STEP_TIMER  4
//   #define MCU_TEMP_TIMER  6
// #endif

// #ifndef STEP_TIMER
//   #define STEP_TIMER MCU_STEP_TIMER
// #endif
// #ifndef TEMP_TIMER
//   #define TEMP_TIMER MCU_TEMP_TIMER
// #endif

// #define __TIMER_DEV(X) TIM##X
// #define _TIMER_DEV(X) __TIMER_DEV(X)
// #define STEP_TIMER_DEV _TIMER_DEV(STEP_TIMER)
// #define TEMP_TIMER_DEV _TIMER_DEV(TEMP_TIMER)

// // --------------------------------------------------------------------------
// // Local defines
// // --------------------------------------------------------------------------

// #define NUM_HARDWARE_TIMERS 1

// // --------------------------------------------------------------------------
// // Private Variables
// // --------------------------------------------------------------------------

// TIM_HandleTypeDef timer_instance[NUM_HARDWARE_TIMERS] = {{}};

// // ------------------------
// // Public functions
// // ------------------------

// uint32_t GetStepperTimerClkFreq() {
//   // Timer input clocks vary between devices, and in some cases between timers on the same device.
//   // Retrieve at runtime to ensure device compatibility. Cache result to avoid repeated overhead.
//   uint32_t clkfreq = 0;
//   #if (STEP_TIMER == 4)
//     clkfreq = HAL_RCC_GetPCLK1Freq();
//     // Double if APB1 prescaler is not 1
//     if ((RCC->D2CFGR & RCC_D2CFGR_D2PPRE1) != RCC_D2CFGR_D2PPRE1_DIV1) {
//       clkfreq *= 2;
//     }
//   #else
//   #error "Change the timer get freq"
//   #endif
//   return clkfreq;
// }

// // frequency is in Hertz
// void HAL_timer_start(const uint8_t timer_num, const uint32_t frequency) {
//   if (!HAL_timer_initialized(timer_num)) {
//     switch (timer_num) {
//       case MF_TIMER_STEP: // STEPPER TIMER - use a 32bit timer if possible
//         memset(&timer_instance[timer_num], 0, sizeof(TIM_HandleTypeDef));
//         timer_instance[timer_num].Instance = STEP_TIMER_DEV;
//         #if (STEP_TIMER == 4)
//         __HAL_RCC_TIM4_CLK_ENABLE();
//         NVIC_SetPriority(TIM4_IRQn, STEP_TIMER_IRQ_PRIO);
//         NVIC_EnableIRQ(TIM4_IRQn);
//         #else
//         #error "Change the timer clock init"
//         #endif
//         /* Set the prescaler to the final desired value.
//          * This will change the effective ISR callback frequency but when
//          * HAL_timer_start(timer_num=0) is called in the core for the first time
//          * the real frequency isn't important as long as, after boot, the ISR
//          * gets called with the correct prescaler and count register. So here
//          * we set the prescaler to the correct, final value and ignore the frequency
//          * asked. We will call back the ISR in 1 second to start at full speed.
//          *
//          * The proper fix, however, would be a correct initialization OR a
//          * HAL_timer_change(const uint8_t timer_num, const uint32_t frequency)
//          * which changes the prescaler when an IRQ frequency change is needed
//          * (for example when steppers are turned on)
//          */

//         timer_instance[timer_num].Init.Prescaler = STEPPER_TIMER_PRESCALE - 1;
//         timer_instance[timer_num].Init.CounterMode = TIM_COUNTERMODE_UP;
//         timer_instance[timer_num].Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
//         timer_instance[timer_num].Init.Period = _MIN(HAL_TIMER_TYPE_MAX, uint32_t((HAL_TIMER_RATE) / (STEPPER_TIMER_PRESCALE) / frequency)) - 1;
//         timer_instance[timer_num].Init.RepetitionCounter = 0;
//         break;
//     }

//     // Disable preload. Leaving it default-enabled can cause the timer to stop if it happens
//     // to exit the ISR after the start time for the next interrupt has already passed.
//     timer_instance[timer_num].Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

//     HAL_timer_enable_interrupt(timer_num);

//     // Start the timer.
//     HAL_TIM_Base_Start_IT(&timer_instance[timer_num]);
//   }
// }

// void HAL_timer_enable_interrupt(const uint8_t timer_num) {
//   if (HAL_timer_initialized(timer_num) && !HAL_timer_interrupt_enabled(timer_num)) {
//     __HAL_TIM_ENABLE_IT(&timer_instance[timer_num], TIM_IT_UPDATE);
//   }
// }

// void HAL_timer_disable_interrupt(const uint8_t timer_num) {
//   __HAL_TIM_DISABLE_IT(&timer_instance[timer_num], TIM_IT_UPDATE);
// }

// bool HAL_timer_interrupt_enabled(const uint8_t timer_num) {
//   switch (timer_num) {
//     case MF_TIMER_STEP:
//       #if (STEP_TIMER == 4)
//       return HAL_timer_initialized(timer_num) && NVIC_GetEnableIRQ(TIM4_IRQn);
//       #else
//       #error "Change the timer IRQ is enabled check"
//       #endif
//       break;
//   }
//   return false;
// }

// void SetTimerInterruptPriorities() {
//   TERN_(HAS_TMC_SW_SERIAL, SoftwareSerial::setInterruptPriority(SWSERIAL_TIMER_IRQ_PRIO, 0));
//   TERN_(HAS_SERVOS, libServo::setInterruptPriority(SERVO_TIMER_IRQ_PRIO, 0));
// }

// // ------------------------
// // Detect timer conflicts
// // ------------------------

// // This list serves two purposes. Firstly, it facilitates build-time mapping between
// // variant-defined timer names (such as TIM1) and timer numbers. It also replicates
// // the order of timers used in the framework's SoftwareSerial.cpp. The first timer in
// // this list will be automatically used by SoftwareSerial if it is not already defined
// // in the board's variant or compiler options.
// static constexpr struct {uintptr_t base_address; int timer_number;} stm32_timer_map[] = {
//   #ifdef TIM18_BASE
//     { uintptr_t(TIM18), 18 },
//   #endif
//   #ifdef TIM7_BASE
//     { uintptr_t(TIM7),   7 },
//   #endif
//   #ifdef TIM6_BASE
//     { uintptr_t(TIM6),   6 },
//   #endif
//   #ifdef TIM22_BASE
//     { uintptr_t(TIM22), 22 },
//   #endif
//   #ifdef TIM21_BASE
//     { uintptr_t(TIM21), 21 },
//   #endif
//   #ifdef TIM17_BASE
//     { uintptr_t(TIM17), 17 },
//   #endif
//   #ifdef TIM16_BASE
//     { uintptr_t(TIM16), 16 },
//   #endif
//   #ifdef TIM15_BASE
//     { uintptr_t(TIM15), 15 },
//   #endif
//   #ifdef TIM14_BASE
//     { uintptr_t(TIM14), 14 },
//   #endif
//   #ifdef TIM13_BASE
//     { uintptr_t(TIM13), 13 },
//   #endif
//   #ifdef TIM11_BASE
//     { uintptr_t(TIM11), 11 },
//   #endif
//   #ifdef TIM10_BASE
//     { uintptr_t(TIM10), 10 },
//   #endif
//   #ifdef TIM12_BASE
//     { uintptr_t(TIM12), 12 },
//   #endif
//   #ifdef TIM19_BASE
//     { uintptr_t(TIM19), 19 },
//   #endif
//   #ifdef TIM9_BASE
//     { uintptr_t(TIM9),   9 },
//   #endif
//   #ifdef TIM5_BASE
//     { uintptr_t(TIM5),   5 },
//   #endif
//   #ifdef TIM4_BASE
//     { uintptr_t(TIM4),   4 },
//   #endif
//   #ifdef TIM3_BASE
//     { uintptr_t(TIM3),   3 },
//   #endif
//   #ifdef TIM2_BASE
//     { uintptr_t(TIM2),   2 },
//   #endif
//   #ifdef TIM20_BASE
//     { uintptr_t(TIM20), 20 },
//   #endif
//   #ifdef TIM8_BASE
//     { uintptr_t(TIM8),   8 },
//   #endif
//   #ifdef TIM1_BASE
//     { uintptr_t(TIM1),   1 }
//   #endif
// };

// // Convert from a timer base address to its integer timer number.
// static constexpr int get_timer_num_from_base_address(uintptr_t base_address) {
//   for (const auto &timer : stm32_timer_map)
//     if (timer.base_address == base_address) return timer.timer_number;
//   return 0;
// }

// // The platform's SoftwareSerial.cpp will use the first timer from stm32_timer_map.
// #if HAS_TMC_SW_SERIAL && !defined(TIMER_SERIAL)
//   #define  TIMER_SERIAL (stm32_timer_map[0].base_address)
// #endif

// // constexpr doesn't like using the base address pointers that timers evaluate to.
// // We can get away with casting them to uintptr_t, if we do so inside an array.
// // GCC will not currently do it directly to a uintptr_t.
// TERN_(HAS_TMC_SW_SERIAL, static constexpr uintptr_t timer_serial[] = {uintptr_t(TIMER_SERIAL)});
// TERN_(SPEAKER,           static constexpr uintptr_t timer_tone[]   = {uintptr_t(TIMER_TONE)});
// TERN_(HAS_SERVOS,        static constexpr uintptr_t timer_servo[]  = {uintptr_t(TIMER_SERVO)});

// enum TimerPurpose { TP_SERIAL, TP_TONE, TP_SERVO, TP_STEP, TP_TEMP };

// // List of timers, to enable checking for conflicts.
// // Includes the purpose of each timer to ease debugging when evaluating at build-time.
// // This cannot yet account for timers used for PWM output, such as for fans.
// static constexpr struct { TimerPurpose p; int t; } timers_in_use[] = {
//   #if HAS_TMC_SW_SERIAL
//     { TP_SERIAL, get_timer_num_from_base_address(timer_serial[0]) }, // Set in variant.h, or as a define in platformio.h if not present in variant.h
//   #endif
//   #if ENABLED(SPEAKER)
//     { TP_TONE, get_timer_num_from_base_address(timer_tone[0]) },     // Set in variant.h, or as a define in platformio.h if not present in variant.h
//   #endif
//   #if HAS_SERVOS
//     { TP_SERVO, get_timer_num_from_base_address(timer_servo[0]) },   // Set in variant.h, or as a define in platformio.h if not present in variant.h
//   #endif
//   { TP_STEP, STEP_TIMER },
//   { TP_TEMP, TEMP_TIMER },
// };

// static constexpr bool verify_no_timer_conflicts() {
//   for (uint8_t i = 0; i < COUNT(timers_in_use); ++i)
//     for (uint8_t j = i + 1; j < COUNT(timers_in_use); ++j)
//       if (timers_in_use[i].t == timers_in_use[j].t) return false;
//   return true;
// }

// // If this assertion fails at compile time, review the timers_in_use array.
// // If default_envs is defined properly in platformio.ini, VS Code can evaluate the array
// // when hovering over it, making it easy to identify the conflicting timers.
// static_assert(verify_no_timer_conflicts(), "One or more timer conflict detected. Examine \"timers_in_use\" to help identify conflict.");

// extern "C" {
//   void TIM4_IRQHandler();
// }

// void TIM4_IRQHandler() {
//   HAL_TIM_IRQHandler(&timer_instance[MF_TIMER_STEP]);
//   Step_Handler();
// }