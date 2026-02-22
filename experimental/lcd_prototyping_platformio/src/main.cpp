#include "stm32f4xx_hal.h"
#include "status.h"
#include "clocks.h"
#include "terminal.h"
#include "lcd.h"
#include "ram.h"
#include "flash.h"
#include "gui.h"
#include "usb.h"
#include "sdmmc/sdmmc.h"
#include "filesystem.h"
#include "file_sender.h"
#include "packet_engine.h"
#include "main.h"

// #include "images/test.h"
// #include "images/squares.h"
#include "images/splashscreen.h"
#include "images/blank.h"
#include "fonts/arial.h"

#include <vector>
#include <stdio.h>
#include <malloc.h>

extern TIM_HandleTypeDef htim3;

int main(void) {
    HAL_Init();

    int ret = clocks_init();
    ret |= terminal_init() << 1;
    ret |= ram_init() << 2;
    ret |= flash_init() << 3;
    ret |= lcd_init() << 4;
    ret |= usb_init() << 5;
    ret |= diskio_init(SD_SPEED_HIGH) << 6;
    ret |= filesystem_init() << 7;

    if (ret == 0) {
        printf("System initialized successfully.\n");
    } else {
        printf("System initialization failed with code: %x\n", ret);
    }

    std::vector<FileInfo> file_list;
    filesystem_get_file_list(file_list);

    lcd_set_background(BLANK);
    lcd_clear_foreground();
    lcd_swap_buffers();

    Scene scene(0xDFFFDF);

    for (size_t i = 0; i < file_list.size(); ++i) {
        auto name_label = std::make_shared<Label>(
            &scene, 10, LCD_HEIGHT - 48 - i * 48, file_list[i].name, 32);
        scene.add_object(name_label);

        auto div_line = std::make_shared<Rectangle>(
            &scene, 10, LCD_HEIGHT - 48 - i * 48 - 4, LCD_WIDTH - 20, 2,
            0x202020);
        scene.add_object(div_line);

        unsigned long long file_size = file_list[i].size;
        char size_str[32];
        if (file_size >= 1024 * 1024) {
            snprintf(size_str, sizeof(size_str), "%.2f MB",
                     file_size / (1024.0 * 1024.0));
        } else if (file_size >= 1024) {
            snprintf(size_str, sizeof(size_str), "%.2f KB", file_size / 1024.0);
        } else {
            snprintf(size_str, sizeof(size_str), "%lu B",
                     (unsigned long)file_size);
        }

        auto size_label = std::make_shared<Label>(
            &scene, LCD_WIDTH - 15, LCD_HEIGHT - 48 - i * 48, size_str, 32);
        size_label->set_right_aligned(true);
        scene.add_object(size_label);
    }

    gui_set_current_scene(&scene);

    uint32_t tick = HAL_GetTick();

    while (1) {
        gui_update();
        gui_render();
        usb_task();
        packet_engine_task();
        file_sender_task();

        if (HAL_GetTick() - tick >= 5000) {
            tick = HAL_GetTick();

            file_sender_send_file("cube.obj");
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