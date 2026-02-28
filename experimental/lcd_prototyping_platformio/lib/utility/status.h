#ifndef STATUS_H
#define STATUS_H

// Adapted from Purdue Space Program High Altitude, Originally written by
// Griffin Ross

#include <stdio.h>

typedef enum {
    STATUS_OK = 0,
    STATUS_ERROR = 1,
    STATUS_BUSY = 2,
    STATUS_TIMEOUT = 3,
} Status;

// Macro abuse :)
#define TRACE_PRINTF(...)                      \
    do {                                       \
        printf("%s:%d: ", __FILE__, __LINE__); \
        printf(__VA_ARGS__);                   \
    } while (0);

#endif  // STATUS_H