#include "lcd.h"

#include "board.h"
#include "gpio/gpio.h"
#include "images/blank.h"
#include "profiler.h"

#include <stdio.h>
#include <string.h>
#include <algorithm>

#define LCD_RESET_PORT GPIOH
#define LCD_RESET_PIN GPIO_PIN_7

LTDC_HandleTypeDef* hltdc;
// DMA2D_HandleTypeDef hdma2d;

// Double-buffered setup
static Color __attribute__((
    section(".fb_ram"))) s_foreground_buffer_0[WINDOW_WIDTH * WINDOW_HEIGHT];
static Color __attribute__((
    section(".fb_ram"))) s_foreground_buffer_1[WINDOW_WIDTH * WINDOW_HEIGHT];

static Color** s_foreground_buffers = (Color*[]){
    s_foreground_buffer_0,
    s_foreground_buffer_1,
};
static volatile int s_current_frontbuffer = 0;

// Sorry about macros
#define FRONTBUFFER s_foreground_buffers[s_current_frontbuffer]
#define BACKBUFFER s_foreground_buffers[1 - s_current_frontbuffer]

Status lcd_init() {
    hltdc = ltdc_get_handle();

    if (ltdc_init() != STATUS_OK) {
        return STATUS_ERROR;
    }

    // __HAL_RCC_DMA2D_CLK_ENABLE();
    // memset(&hdma2d, 0, sizeof(hdma2d));
    // hdma2d.Instance = DMA2D;
    // hdma2d.Init.Mode = DMA2D_R2M;
    // hdma2d.Init.ColorMode = DMA2D_RGB565;
    // hdma2d.Init.OutputOffset = 0;
    // hdma2d.XferCpltCallback = nullptr;
    // hdma2d.XferErrorCallback = nullptr;
    // hdma2d.LayerCfg[0].InputOffset = 0;
    // hdma2d.LayerCfg[0].InputColorMode = DMA2D_RGB565;
    // hdma2d.LayerCfg[0].AlphaMode = DMA2D_NO_MODIF_ALPHA;
    // hdma2d.LayerCfg[0].InputAlpha = 0xFF;
    // hdma2d.LayerCfg[1].InputOffset = 0;
    // hdma2d.LayerCfg[1].InputColorMode = DMA2D_RGB565;
    // hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
    // hdma2d.LayerCfg[1].InputAlpha = 0xFF;

    // // Initialize the DMA2D
    // if (HAL_DMA2D_Init(&hdma2d) != HAL_OK)
    // {
    //     return STATUS_ERROR;
    // }

    // if (HAL_DMA2D_ConfigLayer(&hdma2d, 0) != HAL_OK)
    // {
    //     return STATUS_ERROR;
    // }

    // if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK)
    // {
    //     return STATUS_ERROR;
    // }

    // // LCD init
    // if (nt35510_init(hdsi) != STATUS_OK)
    // {
    //     return STATUS_ERROR;
    // }
    // nt35510_set_brightness(hdsi, 200);

    lcd_set_background(BLANK);

    // Blank both buffers
    memset(s_foreground_buffer_0, 0x00, sizeof(s_foreground_buffer_0));
    memset(s_foreground_buffer_1, 0x00, sizeof(s_foreground_buffer_1));

    HAL_LTDC_SetAddress(hltdc, (uint32_t)FRONTBUFFER, LTDC_LAYER_2);

    // // Touchscreen init
    // // Give some time after reset for the TS driver to become ready
    // HAL_Delay(300);

    // if (ft6336g_init() != STATUS_OK)
    // {
    //     return STATUS_ERROR;
    // }

    // Enable display and backlight
    gpio_write(PIN_LCD_DISP, GPIO_HIGH);
    gpio_write(PIN_BL_EN, GPIO_HIGH);
    gpio_write(PIN_BL_DIM, GPIO_HIGH);

    return STATUS_OK;
}

void lcd_swap_buffers() {
    PROFILER_ENTER();

    // Wait until in vsync to swap buffers
    while (!lcd_is_in_vsync()) {
    }

    // Swap the buffer index
    s_current_frontbuffer = 1 - s_current_frontbuffer;

    // Set the new front buffer and refresh
    lcd_set_foreground(FRONTBUFFER);

    PROFILER_EXIT();
}

Color* lcd_get_backbuffer() {
    return BACKBUFFER;
}

Color* lcd_get_frontbuffer() {
    return FRONTBUFFER;
}

void lcd_set_foreground(const Color* fb_address) {
    HAL_LTDC_SetAddress(hltdc, (uint32_t)fb_address, LTDC_LAYER_2);
}

void lcd_set_background(const ColorBG* fb_address) {
    HAL_LTDC_SetAddress(hltdc, (uint32_t)fb_address, LTDC_LAYER_1);
}

void lcd_clear_foreground(const Color color) {
    memset(BACKBUFFER, color, sizeof(s_foreground_buffer_0));
}

void lcd_clear_area(unsigned int xl, unsigned int xr, unsigned int yb,
                    unsigned int yt) {
    for (unsigned int yi = yb; yi < yt; yi++) {
        for (unsigned int xi = LTDC_WINDOW_WIDTH - xr - 1;
             xi < LTDC_WINDOW_WIDTH - xl; xi++) {
            BACKBUFFER[yi * LTDC_WINDOW_WIDTH + xi] = 0x00;
        }
    }
    // hdma2d.Init.Mode = DMA2D_R2M;
    // hdma2d.Init.OutputOffset = LTDC_WINDOW_HEIGHT - (yt - yb + 1);
    // HAL_DMA2D_Init(&hdma2d);

    // hdma2d.LayerCfg[0].InputOffset = 0;
    // hdma2d.LayerCfg[0].InputColorMode = DMA2D_RGB565;
    // hdma2d.LayerCfg[0].AlphaMode = DMA2D_NO_MODIF_ALPHA;
    // hdma2d.LayerCfg[0].InputAlpha = 0xFF;
    // HAL_DMA2D_ConfigLayer(&hdma2d, 0);

    // HAL_DMA2D_Start(&hdma2d, 0xFFFFFFFF,
    //                 (uint32_t)&BACKBUFFER[yb + xl * LTDC_WINDOW_HEIGHT], (yt
    //                 - yb + 1), (xr - xl + 1));
    // HAL_DMA2D_PollForTransfer(&hdma2d, 100);
}

void lcd_draw_rectangle(unsigned int x, unsigned int y, unsigned int w,
                        unsigned int h, Color color) {
    for (unsigned int yi = y; yi < y + h; yi++) {
        for (unsigned int xi = LTDC_WINDOW_WIDTH - w - x;
             xi < LTDC_WINDOW_WIDTH - x; xi++) {
            BACKBUFFER[yi * LTDC_WINDOW_WIDTH + xi] = color;
        }
    }
    // hdma2d.Init.Mode = DMA2D_R2M;
    // hdma2d.Init.OutputOffset = LTDC_WINDOW_HEIGHT - h;
    // HAL_DMA2D_Init(&hdma2d);

    // hdma2d.LayerCfg[0].InputOffset = 0;
    // hdma2d.LayerCfg[0].InputColorMode = DMA2D_RGB565;
    // hdma2d.LayerCfg[0].AlphaMode = DMA2D_NO_MODIF_ALPHA;
    // hdma2d.LayerCfg[0].InputAlpha = 0xFF;
    // HAL_DMA2D_ConfigLayer(&hdma2d, 0);

    // HAL_DMA2D_Start(&hdma2d, (uint32_t)rgb565_to_rgb888(color),
    //                 (uint32_t)&BACKBUFFER[y + x * LTDC_WINDOW_HEIGHT], h, w);
    // HAL_DMA2D_PollForTransfer(&hdma2d, 100);
}

void lcd_draw_circle(unsigned int x, unsigned int y, unsigned int r,
                     Color color) {
    for (unsigned int yi = y - r; yi < y + r; yi++) {
        for (unsigned int xi = LTDC_WINDOW_WIDTH - x - r;
             xi < LTDC_WINDOW_WIDTH - x + r; xi++) {
            int dx = (int)xi - (int)(LTDC_WINDOW_WIDTH - x);
            int dy = (int)yi - (int)y;

            if ((unsigned int)(dx * dx + dy * dy) < r * r) {
                BACKBUFFER[yi * LTDC_WINDOW_WIDTH + xi] = color;
            }
        }
    }
}

void lcd_copy_background_to_foreground(const Color* fb_address) {
    if (fb_address != NULL) {
        memcpy(BACKBUFFER, fb_address, sizeof(s_foreground_buffer_0));
    } else {
        memcpy(BACKBUFFER, (Color*)hltdc->LayerCfg[0].FBStartAdress,
               sizeof(s_foreground_buffer_0));
    }
}

void lcd_set_foreground_alpha(uint8_t alpha) {
    HAL_LTDC_SetAlpha(hltdc, alpha, LTDC_LAYER_2);
}

void lcd_set_foreground_visibility(bool visible) {
    if (visible) {
        __HAL_LTDC_LAYER_ENABLE(hltdc, LTDC_LAYER_2);
    } else {
        __HAL_LTDC_LAYER_DISABLE(hltdc, LTDC_LAYER_2);
    }
}

bool lcd_is_in_vsync() {
    return (LTDC->CDSR & LTDC_CDSR_VSYNCS) != 0;
}

unsigned int lcd_get_text_width(const Font* font, const char* str,
                                unsigned pt_size) {
    // First check if this size exists
    size_t i;
    for (i = 0; i < font->num_sizes; i++) {
        if (font->sizes[i] == pt_size) {
            break;
        }
    }
    if (i == font->num_sizes) {
        return 0;
    }

    unsigned int width = 0;
    while (*str != '\0') {
        char ch = *str;
        str++;

        if (ch & 0x80) {
            continue;
        }

        const Glyph* glyph = font->glyphs[i][(uint8_t)ch];
        if (glyph->data == NULL) {
            continue;
        }

        width += glyph->advance * pt_size / font->widths[i];
    }

    return width;
}

void lcd_draw_char(const Font* font, char ch, unsigned start_x,
                   unsigned start_y, unsigned pt_size, Color color,
                   unsigned int* advance) {
    if (advance) {
        *advance = 0;
    }

    if (ch & 0x80) {
        return;
    }

    // First check if this size exists
    size_t i;
    for (i = 0; i < font->num_sizes; i++) {
        if (font->sizes[i] == pt_size) {
            break;
        }
    }
    if (i == font->num_sizes) {
        return;
    }

    const Glyph* glyph = font->glyphs[i][(uint8_t)ch];
    int width = font->widths[i];
    int height = font->heights[i];

    if (advance) {
        *advance = glyph->advance * pt_size / width;
    }

    if (glyph->data == NULL) {
        return;
    }

    for (int y = 0; y < (int)pt_size; y++) {
        for (int x = 0; x < (int)pt_size; x++) {
            int dest_x = (LTDC_WINDOW_WIDTH - start_x - x - 1);
            int dest_y = (start_y + pt_size - y);
            if (dest_y < 0 || dest_y >= LTDC_WINDOW_HEIGHT || dest_x < 0 ||
                dest_x >= LTDC_WINDOW_WIDTH) {
                continue;
            }

            // Determine texture coordinates
            int px = x;
            int py = y;

            int offset = px * height + py;
            int offset_byte = offset / 8;
            int offset_bit = offset % 8;

            bool subpixel =
                (glyph->data[offset_byte] & (0x1 << offset_bit)) != 0;
            if (subpixel) {
                BACKBUFFER[(start_y + pt_size - y) * LTDC_WINDOW_WIDTH +
                           (LTDC_WINDOW_WIDTH - start_x - x - 1)] = color;
            }
        }
    }
}

void lcd_draw_text(const Font* font, const char* str, unsigned start_x,
                   unsigned start_y, unsigned pt_size, Color color) {
    unsigned int cur_x = start_x;
    unsigned int advance = 0;
    while (*str != '\0') {
        lcd_draw_char(font, *str, cur_x, start_y, pt_size, color, &advance);
        cur_x += advance;
        str++;
    }
}

void lcd_draw_graphics(const Graphics& graphics, unsigned start_x,
                       unsigned start_y) {
    if (start_x >= LTDC_WINDOW_WIDTH || start_y >= LTDC_WINDOW_HEIGHT) {
        return;
    }
    if (start_x + graphics.width <= 0 || start_y + graphics.height <= 0) {
        return;
    }
    for (unsigned int y = 0; y < graphics.height; y++) {
        for (unsigned int x = 0; x < graphics.width; x++) {
            unsigned int dest_x = LTDC_WINDOW_WIDTH - (start_x + x) - 1;
            unsigned int dest_y = start_y + y;
            if (dest_y >= LTDC_WINDOW_HEIGHT || dest_x >= LTDC_WINDOW_WIDTH) {
                continue;
            }

            if (graphics.data[y * graphics.width + x] == 0x00) {
                continue;
            }

            BACKBUFFER[dest_y * LTDC_WINDOW_WIDTH + dest_x] =
                graphics.data[y * graphics.width + x];
        }
    }
}

static void line_low(int x0, int y0, int x1, int y1, Color color) {
    // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm#All_cases
    /*
    dx = x1 - x0
    dy = y1 - y0
    yi = 1
    if dy < 0
        yi = -1
        dy = -dy
    end if
    D = (2 * dy) - dx
    y = y0

    for x from x0 to x1
        plot(x, y)
        if D > 0
            y = y + yi
            D = D + (2 * (dy - dx))
        else
            D = D + 2*dy
        end if
    */
    int dx = x1 - x0;
    int dy = y1 - y0;
    int yi = 1;
    if (dy < 0) {
        yi = -1;
        dy = -dy;
    }
    int D = (2 * dy) - dx;
    int y = y0;

    for (int x = x0; x <= x1; x++) {
        if (y >= 0 && y < LTDC_WINDOW_HEIGHT && x >= 0 &&
            x < LTDC_WINDOW_WIDTH) {
            BACKBUFFER[y * LTDC_WINDOW_WIDTH + (LTDC_WINDOW_WIDTH - x - 1)] =
                color;
        }
        if (D > 0) {
            y = y + yi;
            D = D + (2 * (dy - dx));
        } else {
            D = D + 2 * dy;
        }
    }
}

static void line_high(int x0, int y0, int x1, int y1, Color color) {
    // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm#All_cases
    /*
    dx = x1 - x0
    dy = y1 - y0
    xi = 1
    if dx < 0
        xi = -1
        dx = -dx
    end if
    D = (2 * dx) - dy
    x = x0

    for y from y0 to y1
        plot(x, y)
        if D > 0
            x = x + xi
            D = D + (2 * (dx - dy))
        else
            D = D + 2*dx
        end if
    */
    int dx = x1 - x0;
    int dy = y1 - y0;
    int xi = 1;
    if (dx < 0) {
        xi = -1;
        dx = -dx;
    }
    int D = (2 * dx) - dy;
    int x = x0;

    for (int y = y0; y <= y1; y++) {
        if (y >= 0 && y < LTDC_WINDOW_HEIGHT && x >= 0 &&
            x < LTDC_WINDOW_WIDTH) {
            BACKBUFFER[y * LTDC_WINDOW_WIDTH + (LTDC_WINDOW_WIDTH - x - 1)] =
                color;
        }
        if (D > 0) {
            x = x + xi;
            D = D + (2 * (dx - dy));
        } else {
            D = D + 2 * dx;
        }
    }
}

void lcd_draw_line(unsigned int x0, unsigned int y0, unsigned int x1,
                   unsigned int y1, Color color) {
    // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm#All_cases
    /*
    if abs(y1 - y0) < abs(x1 - x0)
        if x0 > x1
            plotLineLow(x1, y1, x0, y0)
        else
            plotLineLow(x0, y0, x1, y1)
        end if
    else
        if y0 > y1
            plotLineHigh(x1, y1, x0, y0)
        else
            plotLineHigh(x0, y0, x1, y1)
        end if
    end if
    */
    if (std::abs((int)y1 - (int)y0) < std::abs((int)x1 - (int)x0)) {
        if (x0 > x1) {
            line_low(x1, y1, x0, y0, color);
        } else {
            line_low(x0, y0, x1, y1, color);
        }
    } else {
        if (y0 > y1) {
            line_high(x1, y1, x0, y0, color);
        } else {
            line_high(x0, y0, x1, y1, color);
        }
    }
}