#include "stm32f4xx_hal.h"
#include "status.h"
#include "clocks.h"
#include "terminal.h"
#include "lcd.h"
#include "ram.h"
#include "flash.h"

// #include "images/test.h"
#include "images/squares.h"
// #include "images/splashscreen.h"

#include <stdio.h>

int main(void)
{
  HAL_Init();

  int ret = clocks_init();
  ret |= terminal_init() << 1;
  ret |= ram_init() << 2;
  ret |= flash_init() << 3;
  ret |= lcd_init() << 4;

  lcd_set_background(SQUARES);

  lcd_clear_foreground();

  if (ret == 0)
  {
    printf("System initialized successfully.\n");
  }
  else
  {
    printf("System initialization failed with code: %x\n", ret);
  }

  while (1)
  {
    // int w = 0;
    // while (1)
    // {
    //   if (w == 0)
    //   {
    //     lcd_clear_foreground();
    //   }
    //   lcd_draw_rectangle(49, 101, w, 21, 0xFFFFFFFF);
    //   w = (w + 1) % 703;
    //   HAL_Delay(1);
    // }
  }

  return 0;
}

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
  while (1)
  {
  }
}

void MemManage_Handler(void)
{
  while (1)
  {
  }
}

void BusFault_Handler(void)
{
  while (1)
  {
  }
}

void UsageFault_Handler(void)
{
  while (1)
  {
  }
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
  HAL_IncTick();
}