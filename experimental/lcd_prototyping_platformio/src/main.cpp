#include "main.h"

#include "stm32f4xx_hal.h"
#include "status.h"
#include "clocks.h"
#include "terminal.h"
#include "lcd.h"
#include "ram.h"
#include "flash.h"
#include "gui.h"
#include "gui_app.h"
#include "usb.h"
#include "sdmmc/sdmmc.h"
#include "filesystem.h"
#include "file_sender.h"
#include "packet_engine.h"
#include "profiler.h"

#include <vector>
#include <stdio.h>
#include <malloc.h>

extern TIM_HandleTypeDef htim3;

int main(void) {
    HAL_Init();
    profiler_init();

    int ret = clocks_init();
    ret |= terminal_init() << 1;
    ret |= ram_init() << 2;
    ret |= flash_init() << 3;
    ret |= lcd_init() << 4;
    ret |= usb_init() << 5;
    ret |= diskio_init(SD_SPEED_HIGH) << 6;
    ret |= filesystem_init() << 7;
    ret |= gui_app_init() << 8;

    if (ret == 0) {
        printf("System initialized successfully.\n");
    } else {
        printf("System initialization failed with code: %x\n", ret);
    }

    uint32_t tick = HAL_GetTick();

    while (1) {
        gui_app_task();
        usb_task();
        filesystem_task();
        packet_engine_task();
        file_sender_task();

        if (HAL_GetTick() - tick >= 10000) {
            tick = HAL_GetTick();

            file_sender_send_file("cube.obj");
            profiler_print_summary();
        }
    }

    return 0;
}

// Interrupt handlers mapped to interrupt vectors require C linkage
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
void EXTI9_5_IRQHandler(void);
}

void NMI_Handler(void) {}

extern "C" {
void hardfault_handler_custom(sContextStateFrame* frame);
}

__attribute__((optimize("O0"))) void hardfault_handler_custom(
    sContextStateFrame* frame) {
    HALT_IF_DEBUGGING();
}

void HardFault_Handler(void) {
    HARDFAULT_HANDLING_ASM();
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

extern EXTI_HandleTypeDef g_hexti;
void EXTI9_5_IRQHandler(void) {
    HAL_EXTI_IRQHandler(&g_hexti);

    gui_touch_irq();
}