#ifndef FONT_H
#define FONT_H

#include <stdint.h>

typedef struct Glyph
{
    int advance;
    uint8_t *data;
};

typedef struct
{
    int width;
    int height;
    Glyph glyphs[256];
} Font;

#endif // FONT_H