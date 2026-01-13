#ifndef LTDC_DSI_H
#define LTDC_DSI_H

#include "stm32f4xx_hal.h"

#include "status.h"

#define LTDC_VSYNC 120
#define LTDC_VBP 150
#define LTDC_VFP 150
#define LTDC_HSYNC 2
#define LTDC_HBP 34
#define LTDC_HFP 34
#define LTDC_WIDTH 800
#define LTDC_HEIGHT 480

Status ltdc_dsi_init();
LTDC_HandleTypeDef *ltdc_get_handle();
DSI_HandleTypeDef *dsi_get_handle();

#endif // LTDC_DSI_H