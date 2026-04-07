#ifndef LCD_H
#define LCD_H

#include "status.h"
#include "font.h"
#include "ltdc.h"

#include <stdint.h>
#include <stdbool.h>
#include <array>

#define LCD_WIDTH LTDC_WIDTH
#define LCD_HEIGHT LTDC_HEIGHT

#define WINDOW_WIDTH LTDC_WINDOW_WIDTH
#define WINDOW_HEIGHT LTDC_WINDOW_HEIGHT

typedef uint8_t Color;
typedef uint16_t ColorBG;

Status lcd_init();

void lcd_swap_buffers();
Color *lcd_get_backbuffer();
Color *lcd_get_frontbuffer();

void lcd_set_foreground(const Color *fb_address);
void lcd_set_background(const ColorBG *fb_address);
void lcd_clear_foreground();
void lcd_clear_area(unsigned int xl, unsigned int xr, unsigned int yb,
                    unsigned int yt);
void lcd_draw_rectangle(unsigned int x, unsigned int y, unsigned int w,
                        unsigned int h, Color color);
void lcd_draw_circle(unsigned int x, unsigned int y, unsigned int r,
                     Color color);
void lcd_copy_background_to_foreground(const Color *fb_address);
void lcd_set_foreground_alpha(uint8_t alpha);
void lcd_set_foreground_visibility(bool visible);

bool lcd_is_in_vsync();

unsigned int lcd_get_text_width(const Font *font, const char *str,
                                unsigned pt_size);
void lcd_draw_char(const Font *font, char ch, unsigned start_x,
                   unsigned start_y, unsigned pt_size, Color color,
                   unsigned int *advance);
void lcd_draw_text(const Font *font, const char *str, unsigned start_x,
                   unsigned start_y, unsigned pt_size, Color color);

#endif // LCD_H
