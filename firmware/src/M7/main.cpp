#include "stm32h7xx_hal.h"
#include "clocks.h"
#include "board.h"
#include "gpio/gpio.h"
#include "terminal.h"
#include "flash.h"
#include "sdmmc/sdmmc.h"
#include "ltdc.h"
#include "lcd.h"
#include "fonts/arial.h"
#include "images/blank.h"
#include "gui_app.h"
#include "filesystem.h"
#include "usb.h"
#include "profiler.h"
#include "packet_engine.h"
#include "pi_control.h"
#include "gcode_receiver.h"
#include "scheduler.h"

#include <stdio.h>

Status init();

int main(void) {
    // System initialization
    Status init_stat = init();
    while (init_stat != STATUS_OK) {
        gpio_toggle(PIN_RED);
        HAL_Delay(500);
    }

    // Scheduler setup
    scheduler_add_task("usb", usb_task, 10, 0);
    scheduler_add_task("packet_engine", packet_engine_task, 10, 1);
    scheduler_add_task("gcode_receiver", gcode_receiver_task, 100, 2);
    scheduler_add_task("filesystem", filesystem_task, 100, 3);
    scheduler_add_task("pi_control", pi_control_task, 100, 4);
    scheduler_add_task("gui_app", gui_app_task, 16, 5);

    // Also print the scheduler summary every 10 seconds
    scheduler_add_task("scheduler_summary", scheduler_print_summary, 10000,
                       100);

    // Main loop (just run the scheduler)
    while (1) {
        scheduler_run();
    }

    return 0;
}

/******************/
/* Init Functions */
/******************/

Status corem4_init() {
    HAL_RCCEx_EnableBootCore(RCC_BOOT_C2);
    return STATUS_OK;
}

void gpio_set_initial_state() {
    gpio_write(PIN_TURNTABLE_DIR, GPIO_HIGH);
    gpio_write(PIN_GANTRY_DIR, GPIO_HIGH);
    gpio_write(PIN_EXTRUDER_DIR, GPIO_HIGH);
    gpio_write(PIN_REVOLUTE_DIR, GPIO_HIGH);
}

Status init() {
    HAL_Init();

    int init_stat = STATUS_OK;
    init_stat |= clocks_init() << 0;
    gpio_set_initial_state();
    init_stat |= corem4_init() << 1;
    profiler_init();
    init_stat |= terminal_init() << 2;
    init_stat |= flash_init() << 3;
    init_stat |= filesystem_init() << 4;
    init_stat |= usb_init() << 5;
    init_stat |= lcd_init() << 6;
    init_stat |= gui_app_init() << 7;
    init_stat |= packet_engine_init() << 8;
    init_stat |= scheduler_init() << 9;

    // Give the LCD the status so it can report an error
    gui_app_init_status(init_stat);

    if (init_stat != STATUS_OK) {
        printf("Initialization failed with status: 0x%x\n", init_stat);
        return STATUS_ERROR;
    } else {
        printf("Initialization successful!\n");
    }

    return STATUS_OK;
}

/**********************/
/* Interrupt Handlers */
/**********************/

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