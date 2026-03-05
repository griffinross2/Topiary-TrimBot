#include "ltdc_dsi.h"

#include <stdio.h>

LTDC_HandleTypeDef g_hltdc;
DSI_HandleTypeDef g_hdsi;

Status ltdc_dsi_init() {
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

    if (HAL_LTDC_Init(&g_hltdc) != HAL_OK) {
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

    if (HAL_LTDC_ConfigLayer(&g_hltdc, &pLayerCfg, LTDC_LAYER_1) != HAL_OK) {
        return STATUS_ERROR;
    }

    if (HAL_LTDC_ConfigLayer(&g_hltdc, &pLayerCfg, LTDC_LAYER_2) != HAL_OK) {
        return STATUS_ERROR;
    }

    /************/
    /* DSI Init */
    /************/

    __HAL_RCC_DSI_CLK_ENABLE();

    GPIO_InitTypeDef te_init = {
        .Pin = GPIO_PIN_2,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW,
        .Alternate = GPIO_AF13_DSI,
    };
    HAL_GPIO_Init(GPIOJ, &te_init);

    g_hdsi.Instance = DSI;
    g_hdsi.Init.AutomaticClockLaneControl = DSI_AUTO_CLK_LANE_CTRL_DISABLE;
    g_hdsi.Init.TXEscapeCkdiv = 4;
    g_hdsi.Init.NumberOfLanes = DSI_TWO_DATA_LANES;

    DSI_PLLInitTypeDef PLLInit = {
        .PLLNDIV = 125,
        .PLLIDF = DSI_PLL_IN_DIV2,
        .PLLODF = DSI_PLL_OUT_DIV1,
    };

    if (HAL_DSI_Init(&g_hdsi, &PLLInit) != HAL_OK) {
        return STATUS_ERROR;
    }

    DSI_HOST_TimeoutTypeDef HostTimeouts = {
        .TimeoutCkdiv = 1,
        .HighSpeedTransmissionTimeout = 0,
        .LowPowerReceptionTimeout = 0,
        .HighSpeedReadTimeout = 0,
        .LowPowerReadTimeout = 0,
        .HighSpeedWriteTimeout = 0,
        .HighSpeedWritePrespMode = DSI_HS_PM_DISABLE,
        .LowPowerWriteTimeout = 0,
        .BTATimeout = 0,
    };

    if (HAL_DSI_ConfigHostTimeouts(&g_hdsi, &HostTimeouts) != HAL_OK) {
        return STATUS_ERROR;
    }

    DSI_PHY_TimerTypeDef PhyTimings = {
        .ClockLaneHS2LPTime = 28,
        .ClockLaneLP2HSTime = 33,
        .DataLaneHS2LPTime = 15,
        .DataLaneLP2HSTime = 25,
        .DataLaneMaxReadTime = 0,
        .StopWaitTime = 0,
    };

    if (HAL_DSI_ConfigPhyTimer(&g_hdsi, &PhyTimings) != HAL_OK) {
        return STATUS_ERROR;
    }
    if (HAL_DSI_ConfigFlowControl(&g_hdsi, DSI_FLOW_CONTROL_BTA) != HAL_OK) {
        return STATUS_ERROR;
    }
    if (HAL_DSI_SetLowPowerRXFilter(&g_hdsi, 10000) != HAL_OK) {
        return STATUS_ERROR;
    }
    if (HAL_DSI_ConfigErrorMonitor(&g_hdsi, HAL_DSI_ERROR_NONE) != HAL_OK) {
        return STATUS_ERROR;
    }

    DSI_LPCmdTypeDef LPCmd = {
        .LPGenShortWriteNoP = DSI_LP_GSW0P_DISABLE,
        .LPGenShortWriteOneP = DSI_LP_GSW1P_DISABLE,
        .LPGenShortWriteTwoP = DSI_LP_GSW2P_DISABLE,
        .LPGenShortReadNoP = DSI_LP_GSR0P_DISABLE,
        .LPGenShortReadOneP = DSI_LP_GSR1P_DISABLE,
        .LPGenShortReadTwoP = DSI_LP_GSR2P_DISABLE,
        .LPGenLongWrite = DSI_LP_GLW_DISABLE,
        .LPDcsShortWriteNoP = DSI_LP_DSW0P_DISABLE,
        .LPDcsShortWriteOneP = DSI_LP_DSW1P_DISABLE,
        .LPDcsShortReadNoP = DSI_LP_DSR0P_DISABLE,
        .LPDcsLongWrite = DSI_LP_DLW_DISABLE,
        .LPMaxReadPacket = DSI_LP_MRDP_DISABLE,
        .AcknowledgeRequest = DSI_ACKNOWLEDGE_DISABLE,
    };
    if (HAL_DSI_ConfigCommand(&g_hdsi, &LPCmd) != HAL_OK) {
        return STATUS_ERROR;
    }

    DSI_CmdCfgTypeDef CmdCfg = {
        .VirtualChannelID = 0,
        .ColorCoding = DSI_RGB888,
        .CommandSize = LTDC_WIDTH,
        .TearingEffectSource = DSI_TE_EXTERNAL,
        .TearingEffectPolarity = DSI_TE_RISING_EDGE,
        .HSPolarity = DSI_HSYNC_ACTIVE_LOW,
        .VSPolarity = DSI_VSYNC_ACTIVE_LOW,
        .DEPolarity = DSI_DATA_ENABLE_ACTIVE_HIGH,
        .VSyncPol = DSI_VSYNC_FALLING,
        .AutomaticRefresh = DSI_AR_ENABLE,
        .TEAcknowledgeRequest = DSI_TE_ACKNOWLEDGE_DISABLE,
    };
    if (HAL_DSI_ConfigAdaptedCommandMode(&g_hdsi, &CmdCfg) != HAL_OK) {
        return STATUS_ERROR;
    }

    if (HAL_DSI_ConfigAdaptedCommandMode(&g_hdsi, &CmdCfg) != HAL_OK) {
        return STATUS_ERROR;
    }
    if (HAL_DSI_SetGenericVCID(&g_hdsi, 0) != HAL_OK) {
        return STATUS_ERROR;
    }

    __HAL_DSI_ENABLE_IT(&g_hdsi, DSI_IT_ER);
    __NVIC_EnableIRQ(DSI_IRQn);
    NVIC_SetPriority(DSI_IRQn, 2);

    return STATUS_OK;
}

LTDC_HandleTypeDef* ltdc_get_handle() {
    return &g_hltdc;
}

DSI_HandleTypeDef* dsi_get_handle() {
    return &g_hdsi;
}

extern "C" {
void DSI_IRQHandler(void);
}

void DSI_IRQHandler(void) {
    HAL_DSI_IRQHandler(&g_hdsi);
}