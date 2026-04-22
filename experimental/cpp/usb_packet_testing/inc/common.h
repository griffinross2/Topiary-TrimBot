#pragma once

#include <stdio.h>

// Macro abuse :)
#define TRACE_PRINTF(...)                                \
    do                                                   \
    {                                                    \
        printf("%s:%d: ", __FILE__, __LINE__); \
        printf(__VA_ARGS__);                             \
    } while (0);