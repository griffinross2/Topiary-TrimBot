#ifndef LTDC_DSI_H
#define LTDC_DSI_H

#include "stm32f4xx_hal.h"

#include "status.h"

Status ltdc_dsi_init();
LTDC_HandleTypeDef *ltdc_get_handle();
DSI_HandleTypeDef *dsi_get_handle();

#endif // LTDC_DSI_H