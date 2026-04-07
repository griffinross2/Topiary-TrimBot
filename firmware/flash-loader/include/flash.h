#ifndef FLASH_H
#define FLASH_H

#include "status.h"

Status flash_init();
Status flash_set_memory_mapped_mode();
Status flash_clear_memory_mapped_mode();
Status flash_write(uint32_t addr, uint8_t *buf, int len);
Status flash_read(uint32_t addr, uint8_t *buf, int len);
Status flash_erase_sector(uint32_t addr);
Status flash_erase_chip();

#endif  // FLASH_H