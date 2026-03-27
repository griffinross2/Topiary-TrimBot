#ifndef LTDC_H
#define LTDC_H

#include "stm32h7xx_hal.h"

#include "status.h"

#define LTDC_VSYNC 1
#define LTDC_VBP 3
#define LTDC_VFP 1
#define LTDC_HSYNC 1
#define LTDC_HBP 3
#define LTDC_HFP 1
#define LTDC_WIDTH 800
#define LTDC_HEIGHT 480

Status ltdc_init();
LTDC_HandleTypeDef *ltdc_get_handle();

#endif // LTDC_H