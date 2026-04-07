#include "Dev_Inf.h"

__attribute__((used)) struct StorageInfo __attribute__((section(".Dev_Info"))) const StorageInfo = {
    .DeviceName = "MX25L51245G_STM32H747-TT",
    .DeviceType = NOR_FLASH,
    .DeviceStartAddress = 0x90000000,
    .DeviceSize = 64 * 1024 * 1024,     // 64 MB
    .PageSize = 256,                    // 256 bytes
    .EraseValue = 0xFF,
    .sectors =
        {
            {.SectorNum = 1024, .SectorSize = 64 * 1024}, // 1024 64KB sectors
        },
};