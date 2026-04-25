#ifndef SDMMMC_H
#define SDMMMC_H

// Adapted from Purdue Space Program High Altitude, Originally written by
// Griffin Ross

// Standard library
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// PSPHAA library
#include "status.h"

typedef enum {
    SD_SPEED_INVALID = 0,
    SD_SPEED_DEFAULT = 25000000,
    SD_SPEED_HIGH = 50000000,
} SdmmcSpeed;

// From the HAL states
typedef enum {
    SD_CARD_READY,
    SD_CARD_IDENTIFICATION,
    SD_CARD_STANDBY,
    SD_CARD_TRANSFER,
    SD_CARD_SENDING,
    SD_CARD_RECEIVING,
    SD_CARD_PROGRAMMING,
    SD_CARD_DISCONNECTED,
    SD_CARD_ERROR,
} SdmmcState;

// From the HAL info
typedef struct {
    uint32_t CardType;
    uint32_t CardVersion;
    uint32_t Class;
    uint32_t RelCardAdd;
    uint32_t BlockNbr;
    uint32_t BlockSize;
    uint32_t LogBlockNbr;
    uint32_t LogBlockSize;
    uint32_t CardSpeed;
} SdmmcInfo;

Status diskio_init(SdmmcSpeed clk);

Status sdmmc_init(SdmmcSpeed clk);

Status sdmmc_init_card();

Status sdmmc_write_blocks(uint8_t* tx_buf, uint32_t block_start,
                          uint32_t num_blocks);

Status sdmmc_read_blocks(uint8_t* rx_buf, uint32_t block_start,
                         uint32_t num_blocks);

Status sdmmc_erase(uint32_t block_start, uint32_t block_end);

SdmmcState sdmmc_status();

Status sdmmc_info(SdmmcInfo* info);

#endif  // SDMMMC_H
