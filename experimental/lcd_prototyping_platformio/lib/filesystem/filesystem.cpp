#include "filesystem.h"

#include "ff.h"
#include "sdmmc/sdmmc.h"
#include "timing.h"
#include "gpio/gpio.h"
#include "board.h"

#include "string.h"

static long long unsigned s_last_init_try_tick = 0;
static bool s_inited = false;
static FATFS s_fs;
static FIL s_open_file;

Status filesystem_init() {
    gpio_mode(SD_CARD_DETECT_PIN, GPIO_INPUT_PULLUP);

    if (f_mount(&s_fs, "/", 1) != FR_OK) {
        TRACE_PRINTF("Failed to mount SD card.\n");
    } else {
        TRACE_PRINTF("SD card mounted successfully.\n");
    }

    s_inited = true;

    return STATUS_OK;
}

Status filesystem_get_file_list(std::vector<FileInfo>& file_list) {
    if (!s_inited) {
        file_list.clear();
        return STATUS_OK;
    }

    DIR dir;
    FILINFO finfo;

    if (f_opendir(&dir, "/") != FR_OK) {
        TRACE_PRINTF("Failed to open root directory.\n");
        return STATUS_ERROR;
    }

    while (true) {
        FRESULT res = f_readdir(&dir, &finfo);
        if (res != FR_OK || finfo.fname[0] == 0) {
            break;  // Break on error or end of directory
        }
        if (finfo.fattrib & AM_DIR) {
            continue;  // Skip directories
        }

        file_list.emplace_back(FileInfo{finfo.fname, finfo.fsize});
    }

    f_closedir(&dir);

    return STATUS_OK;
}

Status filesystem_open_file(const char* filename) {
    if (!s_inited) {
        TRACE_PRINTF("Filesystem not initialized.\n");
        return STATUS_ERROR;
    }

    if (f_open(&s_open_file, filename, FA_READ) != FR_OK) {
        TRACE_PRINTF("Failed to open file: %s\n", filename);
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

Status filesystem_read_file(char* buf, size_t len, size_t* actual_len) {
    if (!s_inited) {
        TRACE_PRINTF("Filesystem not initialized.\n");
        return STATUS_ERROR;
    }

    if (f_read(&s_open_file, buf, len, (UINT*)actual_len) != FR_OK) {
        TRACE_PRINTF("Failed to read from file.\n");
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

Status filesystem_seek_file(size_t offset) {
    if (!s_inited) {
        TRACE_PRINTF("Filesystem not initialized.\n");
        return STATUS_ERROR;
    }

    if (f_lseek(&s_open_file, offset) != FR_OK) {
        TRACE_PRINTF("Failed to seek in file.\n");
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

Status filesystem_close_file() {
    if (!s_inited) {
        TRACE_PRINTF("Filesystem not initialized.\n");
        return STATUS_ERROR;
    }

    if (f_close(&s_open_file) != FR_OK) {
        TRACE_PRINTF("Failed to close file.\n");
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

void filesystem_task() {
    GpioValue card_detect = gpio_read(SD_CARD_DETECT_PIN);
    if (s_inited && card_detect == GPIO_HIGH) {
        TRACE_PRINTF("SD card disconnected.\n");
        f_unmount("/");
        memset(&s_fs, 0, sizeof(s_fs));
        s_inited = false;
    } else if (!s_inited && card_detect == GPIO_LOW &&
               get_tick_ms() - s_last_init_try_tick > 500) {
        if (sdmmc_init_card() != STATUS_OK) {
            TRACE_PRINTF("Failed to reinitialize SD card.\n");
            s_last_init_try_tick = get_tick_ms();
            return;
        }
        if (f_mount(&s_fs, "/", 1) == FR_OK) {
            TRACE_PRINTF("SD card reconnected.\n");
            s_inited = true;
        }
        s_last_init_try_tick = get_tick_ms();
    }
}