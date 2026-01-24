#ifndef FONT_H
#define FONT_H

#include <stdint.h>

typedef struct
{
    unsigned int advance;
    const uint8_t *data;
} Glyph;

typedef struct
{
    unsigned int width;
    unsigned int height;
    const Glyph *glyphs[128];
} Font;

#endif // FONT_H