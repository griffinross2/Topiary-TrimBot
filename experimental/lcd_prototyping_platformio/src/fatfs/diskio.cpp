/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2025        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"     /* Basic definitions of FatFs */
#include "diskio.h" /* Declarations FatFs MAI */
#include "stm32f4xx.h"
#include "status.h"
#include "sdmmc/sdmmc.h"

/* MMC/SD command */
#define CMD0 (0)           /* GO_IDLE_STATE */
#define CMD1 (1)           /* SEND_OP_COND (MMC) */
#define ACMD41 (0x80 + 41) /* SEND_OP_COND (SDC) */
#define CMD8 (8)           /* SEND_IF_COND */
#define CMD9 (9)           /* SEND_CSD */
#define CMD10 (10)         /* SEND_CID */
#define CMD12 (12)         /* STOP_TRANSMISSION */
#define ACMD13 (0x80 + 13) /* SD_STATUS (SDC) */
#define CMD16 (16)         /* SET_BLOCKLEN */
#define CMD17 (17)         /* READ_SINGLE_BLOCK */
#define CMD18 (18)         /* READ_MULTIPLE_BLOCK */
#define CMD23 (23)         /* SET_BLOCK_COUNT (MMC) */
#define ACMD23 (0x80 + 23) /* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24 (24)         /* WRITE_BLOCK */
#define CMD25 (25)         /* WRITE_MULTIPLE_BLOCK */
#define CMD32 (32)         /* ERASE_ER_BLK_START */
#define CMD33 (33)         /* ERASE_ER_BLK_END */
#define CMD38 (38)         /* ERASE */
#define CMD55 (55)         /* APP_CMD */
#define CMD58 (58)         /* READ_OCR */

static volatile DSTATUS Stat[FF_VOLUMES] = {
    STA_NOINIT}; /* Physical drive status */

/*-----------------------------------------------------------------------*/
/* CRC functions (from https://github.com/hazelnusse/crc7)               */
/*-----------------------------------------------------------------------*/

static uint8_t s_crc_table[256];

void generate_crc_table() {
    int i, j;
    uint8_t CRCPoly = 0x89;  // the value of our CRC-7 polynomial

    // generate a table value for all 256 possible byte values
    for (i = 0; i < 256; ++i) {
        s_crc_table[i] = (i & 0x80) ? i ^ CRCPoly : i;
        for (j = 1; j < 8; ++j) {
            s_crc_table[i] <<= 1;
            if (s_crc_table[i] & 0x80)
                s_crc_table[i] ^= CRCPoly;
        }
    }
}

// adds a message byte to the current CRC-7 to get a the new CRC-7
uint8_t crc_add(uint8_t crc, uint8_t message_byte) {
    return s_crc_table[(crc << 1) ^ message_byte];
}

// returns the CRC-7 for a message of "length" bytes
uint8_t get_crc(uint8_t message[], int length) {
    int i;
    uint8_t crc = 0;

    for (i = 0; i < length; ++i) {
        crc = crc_add(crc, message[i]);
    }

    return crc;
}

// returns the CRC-7 for a message of "length" bytes
uint8_t get_cmd_crc(BYTE cmd, DWORD arg) {
    uint8_t crc = 0;
    crc = crc_add(crc, cmd | 0x40);        /* Start bit + CMD */
    crc = crc_add(crc, (BYTE)(arg >> 24)); /* Argument[31..24] */
    crc = crc_add(crc, (BYTE)(arg >> 16)); /* Argument[23..16] */
    crc = crc_add(crc, (BYTE)(arg >> 8));  /* Argument[15..8] */
    crc = crc_add(crc, (BYTE)arg);         /* Argument[7..0] */
    return (crc << 1) | 0x01;              /* Set stop bit */
}

/*-----------------------------------------------------------------------*/
/* Peripheral controls (Platform dependent)                              */
/*-----------------------------------------------------------------------*/
static SdmmcSpeed s_sd_clk = SD_SPEED_DEFAULT;

/* Initialize MMC interface */
Status diskio_init(SdmmcSpeed clk) {
    s_sd_clk = clk;

    generate_crc_table();

    return STATUS_OK;
}

/*-----------------------------------------------------------------------*/
/* Check status with timeout                                             */
/*-----------------------------------------------------------------------*/
static Status sdmmc_check_status(uint64_t timeout) {
    SdmmcState res;
    uint64_t start_time = HAL_GetTick();
    do {
        res = sdmmc_status();
    } while ((res != SD_CARD_TRANSFER) &&
             (HAL_GetTick() - start_time < timeout));
    return res == SD_CARD_TRANSFER ? STATUS_OK : STATUS_ERROR;
}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(BYTE drv /* Physical drive number (0) */
) {
    if (drv > 0)
        return STA_NOINIT;

    return Stat[drv]; /* Return disk status */
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(BYTE drv /* Physical drive number (0) */
) {
    if (drv == 0) {
        if (Stat[0] == STA_NOINIT) {
            // Initialize the SDMMC device
            if (Stat[0] & STA_NODISK)
                return Stat[0]; /* Is card existing in the soket? */

            if (sdmmc_init(s_sd_clk) != STATUS_OK) {
                TRACE_PRINTF("Failed to initialize SDMMC hardware\n");
                Stat[0] = STA_NOINIT;
                return Stat[0];
            }
            Stat[0] &= ~STA_NOINIT; /* Clear STA_NOINIT flag */
            return Stat[0];
        }
    } else {
        return STA_NODISK;
    }

    return Stat[0];
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(
    BYTE drv,     /* Physical drive number (0) */
    BYTE* buff,   /* Pointer to the data buffer to store read data */
    LBA_t sector, /* Start sector number (LBA) */
    UINT count    /* Number of sectors to read (1..128) */
) {
    DWORD sect = (DWORD)sector;

    if (drv == 0) {
        while (count--) {
            if (sdmmc_read_blocks((uint8_t*)buff, sect++, 1) != STATUS_OK) {
                return RES_ERROR;
            }
            buff += 512;
        }
        return RES_OK;
    }

    return RES_PARERR;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write(BYTE drv,         /* Physical drive number (0) */
                   const BYTE* buff, /* Ponter to the data to write */
                   LBA_t sector,     /* Start sector number (LBA) */
                   UINT count        /* Number of sectors to write (1..128) */
) {
    DWORD sect = (DWORD)sector;

    if (drv == 0) {
        while (count--) {
            if (sdmmc_write_blocks(&s_sd_sdmmc_device, (uint8_t*)buff, sect++,
                                   1) != STATUS_OK) {
                return RES_ERROR;
            }
            buff += 512;
        }
        return RES_OK;
    }

    return RES_PARERR;
}

#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(BYTE drv,  /* Physical drive number (0) */
                   BYTE cmd,  /* Control command code */
                   void* buff /* Pointer to the conrtol data */
) {
    SdmmcInfo info;
    DWORD st, ed;
    DRESULT res;
    LBA_t* dp;

    if (drv == 0) {
        if (Stat[0] & STA_NOINIT)
            return RES_NOTRDY; /* Check if drive is ready */

        res = RES_ERROR;

        switch (cmd) {
            case CTRL_SYNC: /* Wait for end of internal write process of the
                             * drive
                             */
                if (sdmmc_check_status(200) != STATUS_OK) {
                    res = RES_ERROR;
                    break;
                }
                res = RES_OK;
                break;

            case GET_SECTOR_COUNT: /* Get drive capacity in unit of sector
                                    * (DWORD)
                                    */
                if (sdmmc_info(&info) != STATUS_OK) {
                    res = RES_ERROR;
                    break;
                }
                *(DWORD*)buff = info.LogBlockNbr;
                res = RES_OK;
                break;

            case GET_SECTOR_SIZE: /* Get sector size (WORD) */
                if (sdmmc_info(&info) != STATUS_OK) {
                    res = RES_ERROR;
                    break;
                }
                *(WORD*)buff = info.LogBlockSize;
                res = RES_OK;
                break;

            case GET_BLOCK_SIZE: /* Get erase block size in unit of sector
                                  * (DWORD)
                                  */
                if (sdmmc_info(&info) != STATUS_OK) {
                    res = RES_ERROR;
                    break;
                }
                *(WORD*)buff = info.LogBlockSize / 512;
                res = RES_OK;
                break;

            case CTRL_TRIM: /* Erase a block of sectors (used when _USE_ERASE ==
                             * 1)
                             */
                st = (DWORD)dp[0];
                ed = (DWORD)dp[1];
                if (sdmmc_erase(st, ed) != STATUS_OK) {
                    res = RES_ERROR;
                    break;
                }
                if (sdmmc_check_status(30000) != STATUS_OK) {
                    res = RES_ERROR;
                    break;
                }
                res = RES_OK;
                break;

            default:
                res = RES_PARERR;
        }
        return res;
    }

    return RES_PARERR;
}