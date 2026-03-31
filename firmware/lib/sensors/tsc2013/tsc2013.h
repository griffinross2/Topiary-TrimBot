#ifndef TSC2013_H
#define TSC2013_H

#include "status.h"

#define TSC2013_I2C_ADDRESS 0x48

#define TSC2013_REG_X1 0x00
#define TSC2013_REG_X2 0x01
#define TSC2013_REG_Y1 0x02
#define TSC2013_REG_Y2 0x03
#define TSC2013_REG_IX 0x04
#define TSC2013_REG_IY 0x05
#define TSC2013_REG_Z1 0x06
#define TSC2013_REG_Z2 0x07
#define TSC2013_REG_STATUS 0x08
#define TSC2013_REG_AUX 0x09
#define TSC2013_REG_CFR0 0x0C
#define TSC2013_REG_CFR1 0x0D
#define TSC2013_REG_CFR2 0x0E
#define TSC2013_REG_CFN 0x0F

Status tsc2013_init();
bool tsc2013_is_data_ready();
Status tsc2013_read_touch(uint16_t *x, uint16_t *y, uint16_t *z);
void tsc2013_set_touch_callback(void (*callback)(int tx, int ty));

#endif // TSC2013_H