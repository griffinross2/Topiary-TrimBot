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

    // lcd_draw_text(&ARIAL, "Hello, World!", 50, 150, 80, 0xFF000000);

    // lcd_refresh();

    Scene scene;
    std::shared_ptr<Button> button0 =
        std::make_shared<Button>(&scene, 20, 200, 300, 100);
    std::shared_ptr<Label> label0 =
        std::make_shared<Label>(&scene, 40, 220, "Hello, World!", 48);

    std::shared_ptr<Button> button1 =
        std::make_shared<Button>(&scene, 450, 200, 300, 100);
    std::shared_ptr<Label> label1 =
        std::make_shared<Label>(&scene, 490, 220, "Push Me!", 48);

    scene.add_object(button0);
    scene.add_object(label0);
    scene.add_object(button1);
    scene.add_object(label1);
    gui_set_current_scene(&scene);

    while (1) {
        HAL_Delay(1000);
        printf("Main loop heartbeat.\n");
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