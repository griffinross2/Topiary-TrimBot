#include "stm32h7xx_hal.h"
#include "marlin_wrapper.h"
#include "gpio/gpio.h"
#include "board.h"
#include "gcode/gcode.h"
#include "gcode/queue.h"
#include "module/planner.h"
#include "terminal.h"
#include "profiler.h"
#include "gcode_receiver.h"
#include "door_stop.h"
#include "pi_control.h"

#include <stdio.h>

void main_loop();

int main(void) {
    HAL_Init();

    // Disable cutter pin
    gpio_write(PIN_CUTTER, GPIO_LOW);

    profiler_init();

    int init_stat = 0;
    init_stat |= terminal_rx_start();
    init_stat |= door_stop_init();

    marlin_wrapper_init();

    // Marlin wrapper hosts the loop
    marlin_wrapper_set_idle_cb(main_loop);

    // Delay before homing
    HAL_Delay(500);

    gcode.home_all_axes();
    queue.enqueue_one("G1 Z800");  // Move Z down

    marlin_wrapper_loop();

    return 0;
}

static char s_manual_gcode_buf[64];
static size_t s_manual_gcode_buf_len = 0;
static uint32_t s_profiler_tick = 0;
static uint32_t s_blinky_tick = 0;
static uint32_t s_queue_check_tick = 0;
static uint32_t s_gcode_receive_tick = 0;

void main_loop() {
    // Blink blue LED for status
    if (get_tick_ms() - s_blinky_tick >= 1000) {
        gpio_toggle(PIN_BLU);
        s_blinky_tick = get_tick_ms();
    }

    // Print profiler summary every 10 seconds
    if (get_tick_ms() - s_profiler_tick >= 10000) {
        // profiler_print_summary();
        s_profiler_tick = get_tick_ms();
    }

    // Process manual g-code input
    int bytes_read =
        terminal_read((uint8_t*)(s_manual_gcode_buf + s_manual_gcode_buf_len),
                      sizeof(s_manual_gcode_buf) - 1 - s_manual_gcode_buf_len);
    if (bytes_read > 0) {
        if (s_manual_gcode_buf[s_manual_gcode_buf_len + bytes_read - 1] ==
            '\n') {
            s_manual_gcode_buf[s_manual_gcode_buf_len + bytes_read] = '\0';
            queue.enqueue_one(s_manual_gcode_buf);
            s_manual_gcode_buf_len = 0;
        } else {
            s_manual_gcode_buf_len += bytes_read;
        }
    }

    // Receive g-code from the Pi
    if (get_tick_ms() - s_gcode_receive_tick >= 100) {
        gcode_receiver_task();
        s_gcode_receive_tick = get_tick_ms();
    }

    // Update the status for the Pi
    pi_control_task();

    // Check door status
    door_stop_task();

    if (get_tick_ms() - s_queue_check_tick >= 1000) {
        printf("Queue length: %u\n", queue.ring_buffer.length);
        printf("Currently executing? %s\n", planner.busy() ? "Yes" : "No");
        s_queue_check_tick = get_tick_ms();
    }

    // HAL_Delay(20);
}

extern "C" {
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

void NMI_Handler(void) {
    while (1) {
    }
}

void HardFault_Handler(void) {
    while (1) {
    }
}

void MemManage_Handler(void) {
    while (1) {
    }
}

void BusFault_Handler(void) {
    while (1) {
    }
}

void UsageFault_Handler(void) {
    while (1) {
    }
}

void SVC_Handler(void) {}

void DebugMon_Handler(void) {}

void PendSV_Handler(void) {}

void SysTick_Handler(void) {
    HAL_IncTick();
}