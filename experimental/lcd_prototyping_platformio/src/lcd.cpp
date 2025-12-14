/*
 * lcd.c
 *
 *  Created on: Nov 11, 2025
 *      Author: griff
 */

#include "lcd.h"

#include <stdio.h>
#include <string.h>

#include "ltdc_dsi.h"
#include "nt35510.h"
#include "ft6336g.h"

#define LCD_RESET_PORT GPIOH
#define LCD_RESET_PIN GPIO_PIN_7

DSI_HandleTypeDef* hdsi;
LTDC_HandleTypeDef* hltdc;

static uint32_t __attribute__((
    section(".ext_ram"))) s_foreground_buffer[LCD_WIDTH * LCD_HEIGHT];

Status lcd_init() {
    ltdc_dsi_init();
    hltdc = ltdc_get_handle();
    hdsi = dsi_get_handle();

    GPIO_InitTypeDef lcd_reset = {
        .Pin = LCD_RESET_PIN,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW,
    };

    // Reset the LCD and touchscreen
    HAL_GPIO_Init(LCD_RESET_PORT, &lcd_reset);
    HAL_GPIO_WritePin(LCD_RESET_PORT, LCD_RESET_PIN, GPIO_PIN_SET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(LCD_RESET_PORT, LCD_RESET_PIN, GPIO_PIN_RESET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(LCD_RESET_PORT, LCD_RESET_PIN, GPIO_PIN_SET);
    HAL_Delay(20);

    // LCD init
    if (nt35510_init(hdsi) != STATUS_OK) {
        return STATUS_ERROR;
    }
    nt35510_set_brightness(hdsi, 200);

    lcd_refresh();

    HAL_LTDC_SetAddress(hltdc, (uint32_t)s_foreground_buffer, LTDC_LAYER_2);

    // Touchscreen init
    // Give some time after reset for the TS driver to become ready
    HAL_Delay(300);

    if (ft6336g_init() != STATUS_OK) {
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

uint32_t* lcd_get_framebuffer() {
    return s_foreground_buffer;
}

void lcd_refresh() {
    HAL_DSI_Refresh(hdsi);
}

void lcd_set_background(const uint32_t* fb_address) {
    HAL_DSI_Stop(hdsi);
    __HAL_LTDC_LAYER_DISABLE(hltdc, LTDC_LAYER_1);
    HAL_LTDC_SetAddress(hltdc, (uint32_t)fb_address, LTDC_LAYER_1);
    __HAL_LTDC_LAYER_ENABLE(hltdc, LTDC_LAYER_1);
    HAL_DSI_Start(hdsi);
    lcd_refresh();
}

void lcd_clear_foreground() {
    memset(s_foreground_buffer, 0x00, sizeof(s_foreground_buffer));
    lcd_refresh();
}

void lcd_draw_rectangle(unsigned int x, unsigned int y, unsigned int w,
                        unsigned int h, uint32_t color) {
    for (unsigned int xi = x; xi < x + w; xi++) {
        for (unsigned int yi = y; yi < y + h; yi++) {
            s_foreground_buffer[yi + xi * LCD_HEIGHT] = color;
        }
    }
    lcd_refresh();
}

void lcd_draw_circle(unsigned int x, unsigned int y, unsigned int r,
                     uint32_t color) {
    for (unsigned int xi = x - r; xi < x + r; xi++) {
        for (unsigned int yi = y - r; yi < y + r; yi++) {
            int dx = (int)xi - (int)x;
            int dy = (int)yi - (int)y;

            if (dx * dx + dy * dy < r * r) {
                s_foreground_buffer[yi + xi * LCD_HEIGHT] = color;
            }
        }
    }
    lcd_refresh();
}

void lcd_copy_background_to_foreground(const uint32_t* fb_address) {
    if (fb_address != NULL) {
        memcpy(s_foreground_buffer, fb_address, sizeof(s_foreground_buffer));
    } else {
        // Wait until VSYNC to avoid stepping on the LTDC
        lcd_wait_for_vsync();

        memcpy(s_foreground_buffer, (uint32_t*)hltdc->LayerCfg[0].FBStartAdress,
               sizeof(s_foreground_buffer));
    }
    lcd_refresh();
}

void lcd_set_foreground_alpha(uint8_t alpha) {
    HAL_LTDC_SetAlpha(hltdc, alpha, LTDC_LAYER_2);
    lcd_refresh();
}

void lcd_set_foreground_visibility(bool visible) {
    if (visible) {
        __HAL_LTDC_LAYER_ENABLE(hltdc, LTDC_LAYER_2);
    } else {
        __HAL_LTDC_LAYER_DISABLE(hltdc, LTDC_LAYER_2);
    }
    lcd_refresh();
}

void lcd_wait_for_vsync() {
    while ((hltdc->Instance->CDSR & LTDC_CDSR_VSYNCS) == 0) {
    }
}

void lcd_touch_irq() {
    ft6336g_irq();

    FT6336G_TouchEvent event = ft6336g_get_touch_event(0);
    if (event == FT6336G_TOUCH_EVENT_DOWN ||
        event == FT6336G_TOUCH_EVENT_CONTACT) {
        int x, y, weight;
        if (ft6336g_read_pos(&y, &x, &weight, 0) == STATUS_OK) {
            lcd_draw_circle(x, y, 5, 0xFFFF0000);
        }
    }
}

void lcd_draw_char(const Font* font, char ch, unsigned start_x,
                   unsigned start_y, unsigned pt_size, uint32_t color,
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

    for (int x = 0; x < pt_size; x++) {
        for (int y = 0; y < pt_size; y++) {
            int dest_x = (start_x + x);
            int dest_y = (start_y + pt_size - y);
            if (dest_y < 0 || dest_y >= LCD_HEIGHT || dest_x < 0 ||
                dest_x >= LCD_WIDTH) {
                continue;
            }

            unsigned int pixel_weight = 0;

            for (int mx = 0; mx < TEXT_MULTISAMPLE; mx++) {
                for (int my = 0; my < TEXT_MULTISAMPLE; my++) {
                    // Determine texture coordinates
                    int px = (TEXT_MULTISAMPLE * x + mx) * width / pt_size /
                             TEXT_MULTISAMPLE;
                    int py = (TEXT_MULTISAMPLE * y + my) * height / pt_size /
                             TEXT_MULTISAMPLE;

                    int offset = px * height + py;
                    int offset_byte = offset / 8;
                    int offset_bit = offset % 8;

                    bool subpixel =
                        (glyph->data[offset_byte] & (0x1 << offset_bit)) != 0;
                    if (subpixel) {
                        pixel_weight += 255;
                    }
                }
            }

            pixel_weight /= (TEXT_MULTISAMPLE * TEXT_MULTISAMPLE);
            uint32_t new_color = (pixel_weight << 24) | (color & 0x00FFFFFF);

            lcd_wait_for_vsync();
            s_foreground_buffer[(start_y + pt_size - y) +
                                (start_x + x) * LCD_HEIGHT] = new_color;
        }
    }

    lcd_refresh();
}

void lcd_draw_text(const Font* font, const char* str, unsigned start_x,
                   unsigned start_y, unsigned pt_size, uint32_t color) {
    unsigned int cur_x = start_x;
    unsigned int advance = 0;
    while (*str != '\0') {
        lcd_draw_char(font, *str, cur_x, start_y, pt_size, color, &advance);
        cur_x += advance;
        str++;
    }
}