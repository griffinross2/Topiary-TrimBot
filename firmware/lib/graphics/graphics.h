#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "stdint.h"

typedef struct {
    unsigned int width;
    unsigned int height;
    const uint8_t* data;
} Graphics;

#endif  // GRAPHICS_H