#include "stm32f4xx_hal.h"
#include "status.h"
#include "clocks.h"
#include "terminal.h"
#include "lcd.h"
#include "ram.h"
#include "flash.h"
#include "gui.h"

// #include "images/test.h"
// #include "images/squares.h"
#include "images/splashscreen.h"
#include "images/blank.h"
#include "fonts/arial.h"

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

    if (ret == 0) {
        printf("System initialized successfully.\n");
    } else {
        printf("System initialization failed with code: %x\n", ret);
    }

    // lcd_set_background(SPLASHSCREEN);
    lcd_set_background(BLANK);
    lcd_clear_foreground();
    lcd_request_refresh();

    // lcd_draw_text(&ARIAL, "Hello, World!", 50, 150, 80, 0xFF000000);

    // lcd_refresh();

    Scene scene;
    std::shared_ptr<Button> button0 =
        std::make_shared<Button>(&scene, 10, 100, 150, 50);
    std::shared_ptr<Label> label0 =
        std::make_shared<Label>(&scene, 20, 110, "Hello, World!", 24);

    std::shared_ptr<Button> button1 =
        std::make_shared<Button>(&scene, 225, 100, 150, 50);
    std::shared_ptr<Label> label1 =
        std::make_shared<Label>(&scene, 245, 110, "Push Me!", 24);

    std::shared_ptr<Rectangle> rect0 =
        std::make_shared<Rectangle>(&scene, 0, 0, 25, 25, 0xF5);
    int rect_x = 0;
    int rect_y = 0;

    scene.add_object(button0);
    scene.add_object(label0);
    scene.add_object(button1);
    scene.add_object(label1);
    scene.add_object(rect0);
    gui_set_current_scene(&scene);

    unsigned int tick = HAL_GetTick();
    while (1) {
        gui_update_loop();

        if (HAL_GetTick() - tick >= 10) {
            tick = HAL_GetTick();

            rect_x += 10;
            if (rect_x + 25 >= WINDOW_WIDTH) {
                rect_x = 0;
                rect_y += 10;

                if (rect_y + 25 >= WINDOW_HEIGHT) {
                    rect_y = 0;
                }
            }
            
            rect0->set_position(rect_x, rect_y);
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

extern EXTI_HandleTypeDef g_hexti;
void EXTI9_5_IRQHandler(void) {
    HAL_EXTI_IRQHandler(&g_hexti);
    
    gui_touch_irq();
}