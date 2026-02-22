#include "sdmmc/sdmmc.h"

#include "board.h"
#include "stm32f4xx_hal.h"

SD_HandleTypeDef hsd = {0};
DMA_HandleTypeDef hdma_tx = {0};
DMA_HandleTypeDef hdma_rx = {0};

Status sdmmc_init(SdmmcSpeed clk) {
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
    GPIO_InitStruct.Pin =
        GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    __HAL_RCC_SDMMC1_CLK_ENABLE();
    __HAL_RCC_SDMMC1_FORCE_RESET();
    __HAL_RCC_SDMMC1_RELEASE_RESET();
    NVIC_SetPriority(SDMMC1_IRQn, 0);
    NVIC_EnableIRQ(SDMMC1_IRQn);

    hsd.Instance = SDIO;
    hsd.Init.BusWide = SDIO_BUS_WIDE_1B;
    hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
    hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
    hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
    hsd.Init.ClockBypass = (clk == SD_SPEED_HIGH) ? SDIO_CLOCK_BYPASS_ENABLE
                                                  : SDIO_CLOCK_BYPASS_DISABLE;
    hsd.Init.ClockDiv = 0;  // SDIO_CK = 48MHz / (ClockDiv + 2)

    if (HAL_SD_Init(&hsd) != HAL_OK) {
        return STATUS_ERROR;
    }

    if (hsd.Init.BusWide != SDIO_BUS_WIDE_1B &&
        HAL_SD_ConfigWideBusOperation(&hsd, hsd.Init.BusWide) != HAL_OK) {
        return STATUS_ERROR;
    }

    // DMA configuration
    // __HAL_RCC_DMA2_CLK_ENABLE();
    // NVIC_SetPriority(DMA2_Stream3_IRQn, 0);
    // NVIC_EnableIRQ(DMA2_Stream3_IRQn);

    // hdma_tx.Instance = DMA2_Stream3;
    // hdma_tx.Init.Channel = DMA_CHANNEL_4;
    // hdma_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    // hdma_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    // hdma_tx.Init.MemInc = DMA_MINC_ENABLE;
    // hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    // hdma_tx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    // hdma_tx.Init.Mode = DMA_PFCTRL;
    // hdma_tx.Init.Priority = DMA_PRIORITY_HIGH;
    // hdma_tx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    // hdma_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    // hdma_tx.Init.MemBurst = DMA_MBURST_INC4;
    // hdma_tx.Init.PeriphBurst = DMA_PBURST_INC4;
    // if (HAL_DMA_Init(&hdma_tx) != HAL_OK) {
    //     return STATUS_ERROR;
    // }

    // hdma_rx.Instance = DMA2_Stream3;
    // hdma_rx.Init.Channel = DMA_CHANNEL_4;
    // hdma_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    // hdma_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    // hdma_rx.Init.MemInc = DMA_MINC_ENABLE;
    // hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    // hdma_rx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    // hdma_rx.Init.Mode = DMA_PFCTRL;
    // hdma_rx.Init.Priority = DMA_PRIORITY_HIGH;
    // hdma_rx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    // hdma_rx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    // hdma_rx.Init.MemBurst = DMA_MBURST_INC4;
    // hdma_rx.Init.PeriphBurst = DMA_PBURST_INC4;
    // if (HAL_DMA_Init(&hdma_rx) != HAL_OK) {
    //     return STATUS_ERROR;
    // }

    // __HAL_LINKDMA(&hsd, hdmatx, hdma_tx);
    // __HAL_LINKDMA(&hsd, hdmarx, hdma_rx);

    return STATUS_OK;
}

Status sdmmc_init_card() {
    if (HAL_SD_InitCard(&hsd) != HAL_OK) {
        return STATUS_ERROR;
    }
    return STATUS_OK;
}

Status sdmmc_write_blocks(uint8_t* tx_buf, uint32_t block_start,
                          uint32_t num_blocks) {
    // if (HAL_SD_WriteBlocks_DMA(&hsd, tx_buf, block_start, num_blocks) !=
    //     HAL_OK) {
    //     return STATUS_ERROR;
    // }
    // uint64_t start = HAL_GetTick();
    // while (hsd.State != HAL_SD_STATE_READY ||
    //        HAL_SD_GetCardState(&hsd) != HAL_SD_CARD_TRANSFER) {
    //     if (HAL_GetTick() - start > 500) {
    //         return STATUS_ERROR;
    //     }
    // }
    HAL_SD_WriteBlocks(&hsd, tx_buf, block_start, num_blocks, 5000);
    return STATUS_OK;
}

Status sdmmc_read_blocks(uint8_t* rx_buf, uint32_t block_start,
                         uint32_t num_blocks) {
    // if (HAL_SD_ReadBlocks_DMA(&hsd, rx_buf, block_start, num_blocks) !=
    //     HAL_OK) {
    //     return STATUS_ERROR;
    // }
    // uint64_t start = HAL_GetTick();
    // while (hsd.State != HAL_SD_STATE_READY ||
    //        HAL_SD_GetCardState(&hsd) != HAL_SD_CARD_TRANSFER) {
    //     if (HAL_GetTick() - start > 500) {
    //         return STATUS_ERROR;
    //     }
    // }
    HAL_SD_ReadBlocks(&hsd, rx_buf, block_start, num_blocks, 5000);
    return STATUS_OK;
}

Status sdmmc_erase(uint32_t block_start, uint32_t block_end) {
    if (HAL_SD_Erase(&hsd, block_start, block_end) != HAL_OK) {
        return STATUS_ERROR;
    }
    return STATUS_OK;
}

SdmmcState sdmmc_status() {
    switch (HAL_SD_GetCardState(&hsd)) {
        case HAL_SD_CARD_READY:
            return SD_CARD_READY;
        case HAL_SD_CARD_IDENTIFICATION:
            return SD_CARD_IDENTIFICATION;
        case HAL_SD_CARD_STANDBY:
            return SD_CARD_STANDBY;
        case HAL_SD_CARD_TRANSFER:
            return SD_CARD_TRANSFER;
        case HAL_SD_CARD_SENDING:
            return SD_CARD_SENDING;
        case HAL_SD_CARD_RECEIVING:
            return SD_CARD_RECEIVING;
        case HAL_SD_CARD_PROGRAMMING:
            return SD_CARD_PROGRAMMING;
        case HAL_SD_CARD_DISCONNECTED:
            return SD_CARD_DISCONNECTED;
        case HAL_SD_CARD_ERROR:
            return SD_CARD_ERROR;
        default:
            return SD_CARD_ERROR;
    }
}

Status sdmmc_info(SdmmcInfo* info) {
    info->CardType = hsd.SdCard.CardType;
    info->CardVersion = 0;
    info->Class = hsd.SdCard.Class;
    info->RelCardAdd = hsd.SdCard.RelCardAdd;
    info->BlockNbr = hsd.SdCard.BlockNbr;
    info->BlockSize = hsd.SdCard.BlockSize;
    info->LogBlockNbr = hsd.SdCard.LogBlockNbr;
    info->LogBlockSize = hsd.SdCard.LogBlockSize;
    info->CardSpeed = 0;
    return STATUS_OK;
}

extern "C" {
void SDMMC1_IRQHandler();
void DMA2_Stream3_IRQHandler();
}

void SDMMC1_IRQHandler() {
    HAL_SD_IRQHandler(&hsd);
}

void DMA2_Stream3_IRQHandler() {
    HAL_DMA_IRQHandler(hsd.hdmatx);
    HAL_DMA_IRQHandler(hsd.hdmarx);
}