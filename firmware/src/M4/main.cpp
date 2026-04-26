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
#include "scheduler.h"

#include <stdio.h>

static void main_loop();
static void process_manual_gcode();
static void led_blink();

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

    // Setup the tasks
    scheduler_init();

    scheduler_add_task("door_stop", door_stop_task, 10, 0);
    scheduler_add_task("gcode_receiver", gcode_receiver_task, 100, 1);
    scheduler_add_task("pi_control", pi_control_task, 50, 2);
    scheduler_add_task("led_blink", led_blink, 1000, 3);

    marlin_wrapper_loop();

    return 0;
}

static void main_loop() {
    scheduler_run();
}

static void process_manual_gcode() {
    static char s_manual_gcode_buf[64];
    static size_t s_manual_gcode_buf_len = 0;

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
}

static void led_blink() {
    gpio_toggle(PIN_BLU);
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