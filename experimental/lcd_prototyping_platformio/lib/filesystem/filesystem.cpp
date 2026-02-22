#include "filesystem.h"

#include "ff.h"

static FATFS s_fs;
static FIL s_open_file;

Status filesystem_init() {
    if (f_mount(&s_fs, "/", 1) != FR_OK) {
        TRACE_PRINTF("Failed to mount SD card.\n");
    } else {
        TRACE_PRINTF("SD card mounted successfully.\n");
    }

    return STATUS_OK;
}

Status filesystem_get_file_list(std::vector<FileInfo>& file_list) {
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
    if (f_open(&s_open_file, filename, FA_READ) != FR_OK) {
        TRACE_PRINTF("Failed to open file: %s\n", filename);
        return STATUS_ERROR;
    }
}

Status filesystem_read_file(char* buf, size_t len, size_t* actual_len) {
    if (f_read(&s_open_file, buf, len, (UINT*)actual_len) != FR_OK) {
        TRACE_PRINTF("Failed to read from file.\n");
        return STATUS_ERROR;
    }
}

Status filesystem_seek_file(size_t offset) {
    if (f_lseek(&s_open_file, offset) != FR_OK) {
        TRACE_PRINTF("Failed to seek in file.\n");
        return STATUS_ERROR;
    }
}

Status filesystem_close_file() {
    if (f_close(&s_open_file) != FR_OK) {
        TRACE_PRINTF("Failed to close file.\n");
        return STATUS_ERROR;
    }
}