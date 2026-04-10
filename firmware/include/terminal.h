#ifndef TERMINAL_H
#define TERMINAL_H

#include "status.h"

#define TERMINAL_SEMAPHORE_MAX_WAIT 100

Status terminal_init();
Status terminal_rx_start(); // Called by M4
int terminal_read(uint8_t* buffer, unsigned int size);
void terminal_write(const char *data, unsigned int size);

#endif // TERMINAL_H