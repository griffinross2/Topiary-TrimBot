#include "ltdc.h"

#include "board.h"

#include <stdio.h>

LTDC_HandleTypeDef g_hltdc;

Status ltdc_init()
{
    /**************/
    /* Pin Config */
    /**************/

    /*************/
    /* LTDC Init */
    /*************/

    __HAL_RCC_LTDC_CLK_ENABLE();

    g_hltdc.Instance = LTDC;
    g_hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
    g_hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
    g_hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
    g_hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
    g_hltdc.Init.HorizontalSync = (LTDC_HSYNC - 1);
    g_hltdc.Init.VerticalSync = (LTDC_VSYNC - 1);
    g_hltdc.Init.AccumulatedHBP = (LTDC_HBP + LTDC_HSYNC - 1);
    g_hltdc.Init.AccumulatedVBP = (LTDC_VBP + LTDC_VSYNC - 1);
    g_hltdc.Init.AccumulatedActiveW = (LTDC_HBP + LTDC_WIDTH + LTDC_HSYNC - 1);
    g_hltdc.Init.AccumulatedActiveH = (LTDC_VBP + LTDC_HEIGHT + LTDC_VSYNC - 1);
    g_hltdc.Init.TotalWidth =
        (LTDC_HBP + LTDC_WIDTH + LTDC_HSYNC + LTDC_HFP - 1);
    g_hltdc.Init.TotalHeigh =
        (LTDC_VBP + LTDC_HEIGHT + LTDC_VSYNC + LTDC_VFP - 1);
    g_hltdc.Init.Backcolor.Blue = 255;
    g_hltdc.Init.Backcolor.Green = 255;
    g_hltdc.Init.Backcolor.Red = 255;

    if (HAL_LTDC_Init(&g_hltdc) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    LTDC_LayerCfgTypeDef pLayerCfg = {
        .WindowX0 = 0,
        .WindowX1 = LTDC_WIDTH,
        .WindowY0 = 0,
        .WindowY1 = LTDC_HEIGHT,
        .PixelFormat = LTDC_PIXEL_FORMAT_RGB565,
        .Alpha = 0xFF,
        .Alpha0 = 0xFF,
        .BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA,
        .BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA,
        .FBStartAdress = 0x0,
        .ImageWidth = LTDC_WIDTH,
        .ImageHeight = LTDC_HEIGHT,
        .Backcolor =
            {
                .Blue = 255,
                .Green = 255,
                .Red = 255,
                .Reserved = 0xFF,
            },
    };

    if (HAL_LTDC_ConfigLayer(&g_hltdc, &pLayerCfg, LTDC_LAYER_1) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    if (HAL_LTDC_ConfigLayer(&g_hltdc, &pLayerCfg, LTDC_LAYER_2) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

LTDC_HandleTypeDef *ltdc_get_handle()
{
    return &g_hltdc;
}