#include "stm32f4xx_hal.h"
#include "status.h"
#include "clocks.h"
#include "terminal.h"
#include "lcd.h"
#include "ram.h"
#include "flash.h"

// #include "images/test.h"
// #include "images/squares.h"
#include "images/splashscreen.h"
#include "images/blank.h"
#include "fonts/arial.h"

#include <stdio.h>

int main(void) {
    HAL_Init();

    int ret = clocks_init();
    ret |= terminal_init() << 1;
    ret |= ram_init() << 2;
    ret |= flash_init() << 3;
    ret |= lcd_init() << 4;

    // lcd_set_background(SPLASHSCREEN);
    lcd_set_background(BLANK);

    lcd_clear_foreground();

    lcd_draw_text(&ARIAL, "Hello, World!", 50, 150, 80, 0xFF000000);

    lcd_refresh();

    if (ret == 0) {
        printf("System initialized successfully.\n");
    } else {
        printf("System initialization failed with code: %x\n", ret);
    }

    // Loading bar
    // for (int w = 0; w < 703; w++)
    // {
    //   if (w == 0)
    //   {
    //     lcd_clear_foreground();
    //     lcd_draw_circle(49, 111, 10, 0xFFFFFFFF);
    //   }

    //   lcd_draw_rectangle(49, 101, w, 21, 0xFFFFFFFF);

    //   if (w == 702)
    //   {
    //     lcd_draw_circle(49 + 701, 111, 10, 0xFFFFFFFF);
    //   }

    //   HAL_Delay(2);
    // }

    // // Fade out
    // lcd_copy_background_to_foreground(NULL);
    // lcd_set_background(BLANK);

    // for (int a = 0xFF; a >= 0; a--)
    // {
    //   lcd_set_foreground_alpha(a);
    //   HAL_Delay(1);
    // }

    // lcd_clear_foreground();
    // lcd_set_foreground_alpha(0xFF);

    while (1) {
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
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_5) != RESET) {
        lcd_touch_irq();
    }

    HAL_EXTI_IRQHandler(&g_hexti);
}