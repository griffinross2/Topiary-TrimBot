#ifndef RA8875_H
#define RA8875_H

#include "status.h"
#include <stdint.h>

#define RA8875_ID 0x75
#define RA8875_REG_ID 0x00
#define RA8875_REG_PWRR 0x01
#define RA8875_REG_MRWC 0x02
#define RA8875_REG_PCSR 0x04
#define RA8875_REG_SYSR 0x10

#define RA8875_REG_HDWR 0x14
#define RA8875_REG_HNDFTR 0x15
#define RA8875_REG_HNDR 0x16
#define RA8875_REG_HSTR 0x17
#define RA8875_REG_HPWR 0x18

#define RA8875_REG_VDHR0 0x19
#define RA8875_REG_VDHR1 0x1A
#define RA8875_REG_VNDR0 0x1B
#define RA8875_REG_VNDR1 0x1C
#define RA8875_REG_VSTR0 0x1D
#define RA8875_REG_VSTR1 0x1E
#define RA8875_REG_VPWR 0x1F

#define RA8875_REG_DPCR 0x20

#define RA8875_REG_HSAW0 0x30
#define RA8875_REG_HSAW1 0x31
#define RA8875_REG_VSAW0 0x32
#define RA8875_REG_VSAW1 0x33

#define RA8875_REG_HEAW0 0x34
#define RA8875_REG_HEAW1 0x35
#define RA8875_REG_VEAW0 0x36
#define RA8875_REG_VEAW1 0x37

#define RA8875_REG_LTPR0 0x52
#define RA8875_REG_LTPR1 0x53

#define RA8875_REG_MCLR 0x8E
#define RA8875_REG_MWCR0 0x40
#define RA8875_REG_MWCR1 0x41
#define RA8875_REG_CURH0 0x46
#define RA8875_REG_CURH1 0x47
#define RA8875_REG_CURV0 0x48
#define RA8875_REG_CURV1 0x49

#define RA8875_REG_GPIOX 0xC7

#define RA8875_REG_TPCR0 0x70
#define RA8875_REG_TPCR1 0x71
#define RA8875_REG_PLLC1 0x88
#define RA8875_REG_PLLC2 0x89
#define RA8875_REG_INTC1 0xF0
#define RA8875_REG_INTC2 0xF1

Status ra8875_init();
void ra8875_set_active_layer(unsigned int layer);
void ra8875_set_memory_write_layer(unsigned int layer);
void ra8875_set_memory_write_position(unsigned int x, unsigned int y);
void ra8875_memory_write_byte(uint8_t data);
void ra8875_memory_write_multiple(const uint8_t* data, unsigned int size);
void ra8875_memory_write_multiple(uint8_t value, unsigned int num);
void ra8875_set_pixel(unsigned int x, unsigned int y, uint8_t color);
void ra8875_clear_memory();
void ra8875_draw_rectangle(unsigned int x, unsigned int y, unsigned int w,
                           unsigned int h, uint8_t color);

#endif  // RA8875_H