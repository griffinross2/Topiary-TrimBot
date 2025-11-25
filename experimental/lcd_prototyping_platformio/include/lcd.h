#ifndef LCD_H
#define LCD_H

#include "status.h"
#include <stdint.h>

#define LCD_WIDTH 800
#define LCD_HEIGHT 480

Status lcd_init();

void lcd_set_background(const uint32_t *fb_address);
void lcd_clear_foreground();
void lcd_draw_rectangle(unsigned int x, unsigned int y, unsigned int w, unsigned int h, uint32_t color);

#endif // LCD_H
