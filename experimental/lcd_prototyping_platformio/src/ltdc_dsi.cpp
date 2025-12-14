#include "ltdc_dsi.h"

LTDC_HandleTypeDef g_hltdc;
DSI_HandleTypeDef g_hdsi;

Status ltdc_dsi_init()
{

    /*************/
    /* LTDC Init */
    /*************/

    __HAL_RCC_LTDC_CLK_ENABLE();

    LTDC_LayerCfgTypeDef pLayerCfg = {0};

    g_hltdc.Instance = LTDC;
    g_hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
    g_hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
    g_hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
    g_hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
    g_hltdc.Init.HorizontalSync = 1;
    g_hltdc.Init.VerticalSync = 1;
    g_hltdc.Init.AccumulatedHBP = 3;
    g_hltdc.Init.AccumulatedVBP = 3;
    g_hltdc.Init.AccumulatedActiveW = 803;
    g_hltdc.Init.AccumulatedActiveH = 483;
    g_hltdc.Init.TotalWidth = 805;
    g_hltdc.Init.TotalHeigh = 485;
    g_hltdc.Init.Backcolor.Blue = 255;
    g_hltdc.Init.Backcolor.Green = 255;
    g_hltdc.Init.Backcolor.Red = 255;
    if (HAL_LTDC_Init(&g_hltdc) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    pLayerCfg.WindowX0 = 0;
    pLayerCfg.WindowX1 = 800;
    pLayerCfg.WindowY0 = 0;
    pLayerCfg.WindowY1 = 480;
    pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
    pLayerCfg.Alpha = 0xFF;
    pLayerCfg.Alpha0 = 0xFF;
    pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
    pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
    pLayerCfg.FBStartAdress = 0x0;
    pLayerCfg.ImageWidth = 800;
    pLayerCfg.ImageHeight = 480;
    pLayerCfg.Backcolor.Blue = 255;
    pLayerCfg.Backcolor.Green = 255;
    pLayerCfg.Backcolor.Red = 255;
    if (HAL_LTDC_ConfigLayer(&g_hltdc, &pLayerCfg, LTDC_LAYER_1) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    if (HAL_LTDC_ConfigLayer(&g_hltdc, &pLayerCfg, LTDC_LAYER_2) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    /************/
    /* DSI Init */
    /************/

    __HAL_RCC_DSI_CLK_ENABLE();

    GPIO_InitTypeDef te_init = {0};
    te_init.Pin = GPIO_PIN_2;
    te_init.Mode = GPIO_MODE_AF_PP;
    te_init.Pull = GPIO_NOPULL;
    te_init.Speed = GPIO_SPEED_FREQ_LOW;
    te_init.Alternate = GPIO_AF13_DSI;
    HAL_GPIO_Init(GPIOJ, &te_init);

    DSI_PLLInitTypeDef PLLInit = {0};
    DSI_HOST_TimeoutTypeDef HostTimeouts = {0};
    DSI_PHY_TimerTypeDef PhyTimings = {0};
    DSI_LPCmdTypeDef LPCmd = {0};
    DSI_CmdCfgTypeDef CmdCfg = {0};

    g_hdsi.Instance = DSI;
    g_hdsi.Init.AutomaticClockLaneControl = DSI_AUTO_CLK_LANE_CTRL_DISABLE;
    g_hdsi.Init.TXEscapeCkdiv = 4;
    g_hdsi.Init.NumberOfLanes = DSI_TWO_DATA_LANES;
    PLLInit.PLLNDIV = 125;
    PLLInit.PLLIDF = DSI_PLL_IN_DIV2;
    PLLInit.PLLODF = DSI_PLL_OUT_DIV1;
    if (HAL_DSI_Init(&g_hdsi, &PLLInit) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    HostTimeouts.TimeoutCkdiv = 1;
    HostTimeouts.HighSpeedTransmissionTimeout = 0;
    HostTimeouts.LowPowerReceptionTimeout = 0;
    HostTimeouts.HighSpeedReadTimeout = 0;
    HostTimeouts.LowPowerReadTimeout = 0;
    HostTimeouts.HighSpeedWriteTimeout = 0;
    HostTimeouts.HighSpeedWritePrespMode = DSI_HS_PM_DISABLE;
    HostTimeouts.LowPowerWriteTimeout = 0;
    HostTimeouts.BTATimeout = 0;
    if (HAL_DSI_ConfigHostTimeouts(&g_hdsi, &HostTimeouts) != HAL_OK)
    {
        return STATUS_ERROR;
    }
    PhyTimings.ClockLaneHS2LPTime = 28;
    PhyTimings.ClockLaneLP2HSTime = 33;
    PhyTimings.DataLaneHS2LPTime = 15;
    PhyTimings.DataLaneLP2HSTime = 25;
    PhyTimings.DataLaneMaxReadTime = 0;
    PhyTimings.StopWaitTime = 0;
    if (HAL_DSI_ConfigPhyTimer(&g_hdsi, &PhyTimings) != HAL_OK)
    {
        return STATUS_ERROR;
    }
    if (HAL_DSI_ConfigFlowControl(&g_hdsi, DSI_FLOW_CONTROL_BTA) != HAL_OK)
    {
        return STATUS_ERROR;
    }
    if (HAL_DSI_SetLowPowerRXFilter(&g_hdsi, 10000) != HAL_OK)
    {
        return STATUS_ERROR;
    }
    if (HAL_DSI_ConfigErrorMonitor(&g_hdsi, HAL_DSI_ERROR_NONE) != HAL_OK)
    {
        return STATUS_ERROR;
    }
    LPCmd.LPGenShortWriteNoP = DSI_LP_GSW0P_DISABLE;
    LPCmd.LPGenShortWriteOneP = DSI_LP_GSW1P_DISABLE;
    LPCmd.LPGenShortWriteTwoP = DSI_LP_GSW2P_DISABLE;
    LPCmd.LPGenShortReadNoP = DSI_LP_GSR0P_DISABLE;
    LPCmd.LPGenShortReadOneP = DSI_LP_GSR1P_DISABLE;
    LPCmd.LPGenShortReadTwoP = DSI_LP_GSR2P_DISABLE;
    LPCmd.LPGenLongWrite = DSI_LP_GLW_DISABLE;
    LPCmd.LPDcsShortWriteNoP = DSI_LP_DSW0P_DISABLE;
    LPCmd.LPDcsShortWriteOneP = DSI_LP_DSW1P_DISABLE;
    LPCmd.LPDcsShortReadNoP = DSI_LP_DSR0P_DISABLE;
    LPCmd.LPDcsLongWrite = DSI_LP_DLW_DISABLE;
    LPCmd.LPMaxReadPacket = DSI_LP_MRDP_DISABLE;
    LPCmd.AcknowledgeRequest = DSI_ACKNOWLEDGE_DISABLE;
    if (HAL_DSI_ConfigCommand(&g_hdsi, &LPCmd) != HAL_OK)
    {
        return STATUS_ERROR;
    }
    CmdCfg.VirtualChannelID = 0;
    CmdCfg.ColorCoding = DSI_RGB888;
    CmdCfg.CommandSize = 800;
    CmdCfg.TearingEffectSource = DSI_TE_EXTERNAL;
    CmdCfg.TearingEffectPolarity = DSI_TE_RISING_EDGE;
    CmdCfg.HSPolarity = DSI_HSYNC_ACTIVE_LOW;
    CmdCfg.VSPolarity = DSI_VSYNC_ACTIVE_LOW;
    CmdCfg.DEPolarity = DSI_DATA_ENABLE_ACTIVE_HIGH;
    CmdCfg.VSyncPol = DSI_VSYNC_FALLING;
    CmdCfg.AutomaticRefresh = DSI_AR_ENABLE;
    CmdCfg.TEAcknowledgeRequest = DSI_TE_ACKNOWLEDGE_DISABLE;
    if (HAL_DSI_ConfigAdaptedCommandMode(&g_hdsi, &CmdCfg) != HAL_OK)
    {
        return STATUS_ERROR;
    }
    if (HAL_DSI_SetGenericVCID(&g_hdsi, 0) != HAL_OK)
    {
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

LTDC_HandleTypeDef *ltdc_get_handle()
{
    return &g_hltdc;
}

DSI_HandleTypeDef *dsi_get_handle()
{
    return &g_hdsi;
}