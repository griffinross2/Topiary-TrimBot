#include "ltdc.h"

#include "board.h"
#include "gpio/gpio.h"
#include "images/blank.h"

#include <stdio.h>

LTDC_HandleTypeDef g_hltdc;

Status ltdc_init()
{
    /**************/
    /* Pin Config */
    /**************/

    gpio_mode(PIN_LCD_DISP, GPIO_OUTPUT);
    gpio_mode(PIN_LCD_DE, GPIO_OUTPUT_AF, GPIO_SPD_LOW, 14);
    gpio_mode(PIN_LCD_PCLK, GPIO_OUTPUT_AF, GPIO_SPD_LOW, 14);
    gpio_mode(PIN_LCD_HSYNC, GPIO_OUTPUT_AF, GPIO_SPD_LOW, 14);
    gpio_mode(PIN_LCD_VSYNC, GPIO_OUTPUT_AF, GPIO_SPD_LOW, 14);
    gpio_mode(PIN_LCD_R0, GPIO_OUTPUT_AF, GPIO_SPD_LOW, 14);
    gpio_mode(PIN_LCD_R1, GPIO_OUTPUT_AF, GPIO_SPD_LOW, 14);
    gpio_mode(PIN_LCD_R2, GPIO_OUTPUT_AF, GPIO_SPD_LOW, 14);
    gpio_mode(PIN_LCD_R3, GPIO_OUTPUT_AF, GPIO_SPD_LOW, 9);
    gpio_mode(PIN_LCD_R4, GPIO_OUTPUT_AF, GPIO_SPD_LOW, 14);
    gpio_mode(PIN_LCD_R5, GPIO_OUTPUT_AF, GPIO_SPD_LOW, 14);
    gpio_mode(PIN_LCD_R6, GPIO_OUTPUT_AF, GPIO_SPD_LOW, 9);
    gpio_mode(PIN_LCD_R7, GPIO_OUTPUT_AF, GPIO_SPD_LOW, 14);
    gpio_mode(PIN_LCD_G0, GPIO_OUTPUT_AF, GPIO_SPD_LOW, 14);
    gpio_mode(PIN_LCD_G1, GPIO_OUTPUT_AF, GPIO_SPD_LOW, 14);
    gpio_mode(PIN_LCD_G2, GPIO_OUTPUT_AF, GPIO_SPD_LOW, 14);
    gpio_mode(PIN_LCD_G3, GPIO_OUTPUT_AF, GPIO_SPD_LOW, 14);
    gpio_mode(PIN_LCD_G4, GPIO_OUTPUT_AF, GPIO_SPD_LOW, 14);
    gpio_mode(PIN_LCD_G5, GPIO_OUTPUT_AF, GPIO_SPD_LOW, 14);
    gpio_mode(PIN_LCD_G6, GPIO_OUTPUT_AF, GPIO_SPD_LOW, 14);
    gpio_mode(PIN_LCD_G7, GPIO_OUTPUT_AF, GPIO_SPD_LOW, 14);
    gpio_mode(PIN_LCD_B0, GPIO_OUTPUT_AF, GPIO_SPD_LOW, 14);
    gpio_mode(PIN_LCD_B1, GPIO_OUTPUT_AF, GPIO_SPD_LOW, 14);
    gpio_mode(PIN_LCD_B2, GPIO_OUTPUT_AF, GPIO_SPD_LOW, 14);
    gpio_mode(PIN_LCD_B3, GPIO_OUTPUT_AF, GPIO_SPD_LOW, 14);
    gpio_mode(PIN_LCD_B4, GPIO_OUTPUT_AF, GPIO_SPD_LOW, 14);
    gpio_mode(PIN_LCD_B5, GPIO_OUTPUT_AF, GPIO_SPD_LOW, 14);
    gpio_mode(PIN_LCD_B6, GPIO_OUTPUT_AF, GPIO_SPD_LOW, 14);
    gpio_mode(PIN_LCD_B7, GPIO_OUTPUT_AF, GPIO_SPD_LOW, 14);

    /*************/
    /* LTDC Init */
    /*************/

    __HAL_RCC_LTDC_CLK_ENABLE();

    g_hltdc.Instance = LTDC;
    g_hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
    g_hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
    g_hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AH;
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

    LTDC_LayerCfgTypeDef pLayer1Cfg = {
        .WindowX0 = 0,
        .WindowX1 = LTDC_WIDTH,
        .WindowY0 = 0,
        .WindowY1 = LTDC_HEIGHT,
        .PixelFormat = LTDC_PIXEL_FORMAT_RGB565,
        .Alpha = 0xFF,
        .Alpha0 = 0xFF,
        .BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA,
        .BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA,
        .FBStartAdress = (uint32_t)BLANK,
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

    LTDC_LayerCfgTypeDef pLayer2Cfg = {
        .WindowX0 = (LTDC_WIDTH - LTDC_WINDOW_WIDTH) / 2,
        .WindowX1 = ((LTDC_WIDTH - LTDC_WINDOW_WIDTH) / 2) + LTDC_WINDOW_WIDTH - 1,
        .WindowY0 = (LTDC_HEIGHT - LTDC_WINDOW_HEIGHT) / 2,
        .WindowY1 = ((LTDC_HEIGHT - LTDC_WINDOW_HEIGHT) / 2) + LTDC_WINDOW_HEIGHT - 1,
        .PixelFormat = LTDC_PIXEL_FORMAT_L8,
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

    if (HAL_LTDC_ConfigLayer(&g_hltdc, &pLayer1Cfg, LTDC_LAYER_1) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    if (HAL_LTDC_ConfigLayer(&g_hltdc, &pLayer2Cfg, LTDC_LAYER_2) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

LTDC_HandleTypeDef *ltdc_get_handle()
{
    return &g_hltdc;
}