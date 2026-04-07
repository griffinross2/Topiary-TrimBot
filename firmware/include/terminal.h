#ifndef TERMINAL_H
#define TERMINAL_H

#include "status.h"

#define TERMINAL_SEMAPHORE_MAX_WAIT 100

Status terminal_init();
void terminal_write(const char *data, unsigned int size);

#endif // TERMINAL_H