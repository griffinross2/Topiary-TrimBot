#ifndef FONT_H
#define FONT_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    unsigned int advance;
    const uint8_t* data;
} Glyph;

typedef struct {
    size_t num_sizes;
    unsigned int sizes[16];
    unsigned int widths[16];
    unsigned int heights[16];
    const Glyph* glyphs[16][128];
} Font;

#endif  // FONT_H