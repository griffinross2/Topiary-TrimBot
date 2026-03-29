#ifndef TERMINAL_H
#define TERMINAL_H

#include "status.h"

Status terminal_init();
void terminal_write(const char *data, unsigned int size);

#endif // TERMINAL_H