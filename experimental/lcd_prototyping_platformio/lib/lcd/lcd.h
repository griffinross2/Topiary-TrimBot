#ifndef LCD_H
#define LCD_H

#include "status.h"
#include "font.h"

#include <stdint.h>
#include <stdbool.h>
#include <array>

#define LCD_WIDTH 800
#define LCD_HEIGHT 480

#define WINDOW_WIDTH 480
#define WINDOW_HEIGHT 272

typedef uint32_t ColorRGB888;
typedef uint16_t ColorRGB565;

#define LITERAL_RGB888_TO_RGB565(rgb888)                       \
    ((((rgb888) >> 8) & 0xF800) | (((rgb888) >> 5) & 0x07E0) | \
     (((rgb888) >> 3) & 0x001F))
ColorRGB565 rgb888_to_rgb565(ColorRGB888);

Status lcd_init();

void lcd_refresh();

void lcd_swap_buffers();
ColorRGB565* lcd_get_backbuffer();
ColorRGB565* lcd_get_frontbuffer();

void lcd_set_foreground(const ColorRGB565* fb_address);
void lcd_set_background(const ColorRGB565* fb_address);
void lcd_clear_foreground();
void lcd_clear_area(unsigned int xl, unsigned int xr, unsigned int yb,
                    unsigned int yt);
void lcd_draw_rectangle(unsigned int x, unsigned int y, unsigned int w,
                        unsigned int h, ColorRGB565 color);
void lcd_draw_circle(unsigned int x, unsigned int y, unsigned int r,
                     ColorRGB565 color);
void lcd_copy_background_to_foreground(const ColorRGB565* fb_address);
void lcd_set_foreground_alpha(uint8_t alpha);
void lcd_set_foreground_visibility(bool visible);

void lcd_wait_for_vsync();

void lcd_draw_char(const Font* font, char ch, unsigned start_x,
                   unsigned start_y, unsigned pt_size, ColorRGB565 color,
                   unsigned int* advance);
void lcd_draw_text(const Font* font, const char* str, unsigned start_x,
                   unsigned start_y, unsigned pt_size, ColorRGB565 color);

#endif  // LCD_H
