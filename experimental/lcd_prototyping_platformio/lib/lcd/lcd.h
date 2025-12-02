#ifndef LCD_H
#define LCD_H

#include "status.h"
#include <stdint.h>

#define LCD_WIDTH 800
#define LCD_HEIGHT 480

Status lcd_init();

uint32_t* lcd_get_framebuffer();
void lcd_set_background(const uint32_t *fb_address);
void lcd_clear_foreground();
void lcd_draw_rectangle(unsigned int x, unsigned int y, unsigned int w, unsigned int h, uint32_t color);
void lcd_draw_circle(unsigned int x, unsigned int y, unsigned int r, uint32_t color);
void lcd_copy_background_to_foreground();
void lcd_set_foreground_alpha(uint8_t alpha);

#endif // LCD_H
