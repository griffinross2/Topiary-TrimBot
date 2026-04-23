#include "stm32h7xx_hal.h"
#include "clocks.h"
#include "board.h"
#include "gpio/gpio.h"
#include "terminal.h"
#include "flash.h"
#include "sdmmc/sdmmc.h"
#include "tsc2013/tsc2013.h"
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

#include <stdio.h>

Status corem4_init() {
    HAL_RCCEx_EnableBootCore(RCC_BOOT_C2);
    return STATUS_OK;
}

int main(void) {
    HAL_Init();

    int init_stat = STATUS_OK;
    init_stat |= clocks_init() << 0;
    init_stat |= corem4_init() << 1;
    profiler_init();
    init_stat |= terminal_init() << 2;
    init_stat |= flash_init() << 3;
    init_stat |= filesystem_init() << 4;
    init_stat |= usb_init() << 5;
    init_stat |= tsc2013_init() << 6;
    init_stat |= lcd_init() << 7;
    init_stat |= gui_app_init() << 8;
    init_stat |= packet_engine_init() << 9;

    gui_app_init_status(init_stat);

    printf("Init status: 0x%x\n", init_stat);

    if (init_stat != STATUS_OK) {
        gpio_write(PIN_RED, GPIO_HIGH);
    } else {
        gpio_write(PIN_GRN, GPIO_HIGH);
    }

    gpio_write(PIN_TURNTABLE_DIR, GPIO_HIGH);
    gpio_write(PIN_GANTRY_DIR, GPIO_HIGH);
    gpio_write(PIN_EXTRUDER_DIR, GPIO_HIGH);
    gpio_write(PIN_REVOLUTE_DIR, GPIO_HIGH);

    std::vector<FileInfo> file_list;
    filesystem_get_file_list(file_list);
    for (size_t i = 0; i < file_list.size(); i++) {
        printf("File %lu: %s (%lu bytes)\n", (uint32_t)i,
               file_list[i].name.c_str(), (uint32_t)file_list[i].size);
    }

    uint32_t profiler_tick = HAL_GetTick();
    uint32_t s_gcode_receive_tick = HAL_GetTick();

    // Main loop
    while (1) {
        // Print profiler summary every 10 seconds
        if (HAL_GetTick() - profiler_tick >= 10000) {
            profiler_print_summary();
            profiler_tick = HAL_GetTick();
        }

        filesystem_task();
        usb_task();
        packet_engine_task();
        gui_app_task();
        pi_control_task();

        if (HAL_GetTick() - s_gcode_receive_tick >= 100) {
            gcode_receiver_task();
            s_gcode_receive_tick = HAL_GetTick();
        }
    }

    return 0;
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