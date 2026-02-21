#pragma once

#include <stdio.h>

// Macro abuse :)
#define FILENAME(file) (strrchr(file, '/') ? strrchr(file, '/') + 1 : (strrchr(file, '\\') ? strrchr(file, '\\') + 1 : file))
#define TRACE_PRINTF(...)                                \
    do                                                   \
    {                                                    \
        printf("%s:%d: ", FILENAME(__FILE__), __LINE__); \
        printf(__VA_ARGS__);                             \
    } while (0);