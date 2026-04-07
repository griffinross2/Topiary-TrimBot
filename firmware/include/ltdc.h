#ifndef LTDC_H
#define LTDC_H

#include "stm32h7xx_hal.h"

#include "status.h"

// 7-inch display parameters
// https://cdn-shop.adafruit.com/product-files/2354/Datasheet+.pdf

// 33.33 MHz pixel clock

// Active width = 800
// HSYNC width = 1
// HBP (excluding HSYNC) = 45
// HFP = 210
// Total width = 800 + 1 + 45 + 210 = 1056

// Active height = 480
// VSYNC width = 1
// VBP (excluding VSYNC) = 22
// VFP = 22
// Total height = 480 + 1 + 22 + 22 = 525

// Window width = 800
// Window height = 326
// 800 * 326 * 8bpp * 2 buffers ~ 509 kB to fit in RAM_D1

#define LTDC_VSYNC 1
#define LTDC_VBP 22
#define LTDC_VFP 22
#define LTDC_HSYNC 1
#define LTDC_HBP 45
#define LTDC_HFP 210
#define LTDC_WIDTH 800
#define LTDC_HEIGHT 480
#define LTDC_WINDOW_WIDTH LTDC_WIDTH
#define LTDC_WINDOW_HEIGHT 326

Status ltdc_init();
LTDC_HandleTypeDef *ltdc_get_handle();

#endif // LTDC_H