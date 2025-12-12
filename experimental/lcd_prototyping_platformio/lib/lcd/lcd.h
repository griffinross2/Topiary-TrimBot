#ifndef LCD_H
#define LCD_H

#include "status.h"
#include <stdint.h>
#include <stdbool.h>

#define LCD_WIDTH 800
#define LCD_HEIGHT 480

Status lcd_init();

uint32_t *lcd_get_framebuffer();
void lcd_set_background(const uint32_t *fb_address);
void lcd_clear_foreground();
void lcd_draw_rectangle(unsigned int x, unsigned int y, unsigned int w, unsigned int h, uint32_t color);
void lcd_draw_circle(unsigned int x, unsigned int y, unsigned int r, uint32_t color);
void lcd_copy_background_to_foreground(const uint32_t *fb_address);
void lcd_set_foreground_alpha(uint8_t alpha);
void lcd_set_foreground_visibility(bool visible);
void lcd_wait_for_vsync();
void lcd_touch_irq();

#endif // LCD_H
