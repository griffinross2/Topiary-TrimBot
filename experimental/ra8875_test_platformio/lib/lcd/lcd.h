#ifndef LCD_H
#define LCD_H

#include "status.h"
#include "font.h"

#include <stdint.h>
#include <stdbool.h>
#include <array>

#define LCD_WIDTH 800
#define LCD_HEIGHT 480

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 480

typedef uint32_t ColorRGB888;
typedef uint8_t ColorRGB332;

#define LITERAL_RGB888_TO_RGB332(rgb888)                     \
    ((((rgb888) >> 16) & 0xE0) | (((rgb888) >> 11) & 0x1C) | \
     (((rgb888) >> 6) & 0x03))
ColorRGB332 rgb888_to_rgb332(ColorRGB888);

Status lcd_init();

void lcd_refresh();

void lcd_swap_buffers();

void lcd_clear_foreground();
void lcd_clear_area(unsigned int xl, unsigned int xr, unsigned int yb,
                    unsigned int yt);
void lcd_draw_rectangle(unsigned int x, unsigned int y, unsigned int w,
                        unsigned int h, ColorRGB332 color);
void lcd_draw_circle(unsigned int x, unsigned int y, unsigned int r,
                     ColorRGB332 color);
void lcd_set_foreground_alpha(uint8_t alpha);
void lcd_set_foreground_visibility(bool visible);

void lcd_wait_for_vsync();

void lcd_draw_char(const Font* font, char ch, unsigned start_x,
                   unsigned start_y, unsigned pt_size, ColorRGB332 color,
                   unsigned int* advance);
void lcd_draw_text(const Font* font, const char* str, unsigned start_x,
                   unsigned start_y, unsigned pt_size, ColorRGB332 color);

#endif  // LCD_H
