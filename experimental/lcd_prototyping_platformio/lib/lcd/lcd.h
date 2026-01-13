#ifndef LCD_H
#define LCD_H

#include "status.h"
#include "font.h"

#include <stdint.h>
#include <stdbool.h>

#define LCD_WIDTH 800
#define LCD_HEIGHT 480

#define WINDOW_WIDTH 480
#define WINDOW_HEIGHT 272

typedef uint8_t Color;

Status lcd_init();
void lcd_refresh();

uint8_t* lcd_get_framebuffer();
void lcd_set_background(const uint8_t* fb_address);
void lcd_clear_foreground();
void lcd_clear_area(unsigned int xl, unsigned int xr,
                    unsigned int yb, unsigned int yt);
void lcd_draw_rectangle(unsigned int x, unsigned int y, unsigned int w,
                        unsigned int h, Color color);
void lcd_draw_circle(unsigned int x, unsigned int y, unsigned int r,
                     Color color);
void lcd_copy_background_to_foreground(const uint32_t* fb_address);
void lcd_set_foreground_alpha(uint8_t alpha);
void lcd_set_foreground_visibility(bool visible);

void lcd_wait_for_vsync();

void lcd_draw_char(const Font* font, char ch, unsigned start_x,
                   unsigned start_y, unsigned pt_size, Color color,
                   unsigned int* advance);
void lcd_draw_text(const Font* font, const char* str, unsigned start_x,
                   unsigned start_y, unsigned pt_size, Color color);

#endif  // LCD_H
