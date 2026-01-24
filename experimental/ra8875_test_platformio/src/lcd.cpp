/*
 * lcd.c
 *
 *  Created on: Nov 11, 2025
 *      Author: griff
 */

#include "lcd.h"

#include <stdio.h>
#include <string.h>
#include <algorithm>

#include "ra8875.h"

static volatile int s_current_frontbuffer = 0;

ColorRGB332 rgb888_to_rgb332(ColorRGB888 color) {
    return ((color >> 16) & 0xE0) | ((color >> 11) & 0x1C) |
           ((color >> 6) & 0x03);
}

Status lcd_init() {
    if (ra8875_init() != STATUS_OK) {
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

void lcd_swap_buffers() {
    // Swap the buffer index
    s_current_frontbuffer = 1 - s_current_frontbuffer;

    // Set the new front buffer and back buffer
    ra8875_set_active_layer(s_current_frontbuffer);
    ra8875_set_memory_write_layer(1 - s_current_frontbuffer);
}

void lcd_refresh() {}

void lcd_clear_foreground() {
    ra8875_clear_memory();
}

void lcd_clear_area(unsigned int xl, unsigned int xr, unsigned int yb,
                    unsigned int yt) {
    for (unsigned int xi = xl; xi <= xr; xi++) {
        for (unsigned int yi = yb; yi <= yt; yi++) {
            ra8875_set_pixel(xi, yi, 0x00);
        }
    }
}

void lcd_draw_rectangle(unsigned int x, unsigned int y, unsigned int w,
                        unsigned int h, ColorRGB332 color) {
    // for (unsigned int xi = x; xi < x + w; xi++) {
    //     for (unsigned int yi = y; yi < y + h; yi++) {
    //         ra8875_set_pixel(xi, yi, color);
    //     }
    // }
    ra8875_draw_rectangle(x, y, w, h, color);
}

void lcd_draw_circle(unsigned int x, unsigned int y, unsigned int r,
                     ColorRGB332 color) {
    for (unsigned int xi = x - r; xi < x + r; xi++) {
        for (unsigned int yi = y - r; yi < y + r; yi++) {
            int dx = (int)xi - (int)x;
            int dy = (int)yi - (int)y;

            if ((unsigned int)(dx * dx + dy * dy) < r * r) {
                ra8875_set_pixel(xi, yi, color);
            }
        }
    }
}

void lcd_set_foreground_alpha(uint8_t alpha) {}

void lcd_set_foreground_visibility(bool visible) {}

void lcd_wait_for_vsync() {}

void lcd_draw_char(const Font* font, char ch, unsigned start_x,
                   unsigned start_y, unsigned pt_size, ColorRGB332 color,
                   unsigned int* advance) {
    if (advance) {
        *advance = 0;
    }

    if (ch < 0) {
        return;
    }

    const Glyph* glyph = font->glyphs[(uint8_t)ch];
    int width = font->width;
    int height = font->height;

    if (advance) {
        *advance = glyph->advance * pt_size / width;
    }

    if (glyph->data == NULL) {
        return;
    }

    for (int x = 0; x < (int)pt_size; x++) {
        for (int y = 0; y < (int)pt_size; y++) {
            int dest_x = (start_x + x);
            int dest_y = (start_y + y);
            if (dest_y < 0 || dest_y >= LCD_HEIGHT || dest_x < 0 ||
                dest_x >= LCD_WIDTH) {
                continue;
            }

            // Determine texture coordinates
            int px = x * width / pt_size;
            int py = y * height / pt_size;

            int offset = px * height + py;
            int offset_byte = offset / 8;
            int offset_bit = offset % 8;

            bool subpixel =
                (glyph->data[offset_byte] & (0x1 << offset_bit)) != 0;
            if (subpixel) {
                ra8875_set_pixel((start_x + x), (start_y + y), color);
            }
        }
    }
}

void lcd_draw_text(const Font* font, const char* str, unsigned start_x,
                   unsigned start_y, unsigned pt_size, ColorRGB332 color) {
    unsigned int cur_x = start_x;
    unsigned int advance = 0;
    while (*str != '\0') {
        lcd_draw_char(font, *str, cur_x, start_y, pt_size, color, &advance);
        cur_x += advance;
        str++;
    }
}
