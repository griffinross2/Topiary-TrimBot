#ifndef STATUS_H
#define STATUS_H

// Adapted from Purdue Space Program High Altitude, Originally written by
// Griffin Ross

#include <stdio.h>

typedef enum
{
    STATUS_OK = 0,
    STATUS_ERROR = 1,
    STATUS_BUSY = 2,
    STATUS_TIMEOUT = 3,
    STATUS_PARAMETER_ERROR = 4,
} Status;

#endif // STATUS_H