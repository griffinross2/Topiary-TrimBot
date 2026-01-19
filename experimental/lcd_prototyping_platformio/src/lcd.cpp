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

#include "ltdc_dsi.h"
#include "nt35510.h"
#include "ft6336g.h"

#define LCD_RESET_PORT GPIOH
#define LCD_RESET_PIN GPIO_PIN_7

DSI_HandleTypeDef* hdsi;
LTDC_HandleTypeDef* hltdc;
DMA_HandleTypeDef hdma = {0};

// Double-buffered setup
static uint8_t __attribute__((
    section(".ext_ram"))) s_foreground_buffer_0[LCD_WIDTH * LCD_HEIGHT];
static uint8_t __attribute__((
    section(".ext_ram"))) s_foreground_buffer_1[LCD_WIDTH * LCD_HEIGHT];

static uint8_t** s_foreground_buffers = (uint8_t*[]){
    s_foreground_buffer_0,
    s_foreground_buffer_1,
};
static volatile int s_current_frontbuffer = 0;
static volatile bool s_lock_buffer = false;

// Sorry about macros
#define FRONTBUFFER s_foreground_buffers[s_current_frontbuffer]
#define BACKBUFFER s_foreground_buffers[1 - s_current_frontbuffer]

// EOF callback
void lcd_end_of_refresh_callback(DSI_HandleTypeDef* hdsi);

// DMA transfer complete callback
void lcd_dma_tc_callback(DMA_HandleTypeDef* hdma);

// Refresh request flag
static volatile bool s_refresh_req = false;

// Bytes transferred so far during buffer swap
static volatile unsigned long s_buffer_swap_bytes = 0;

static volatile unsigned long s_swap_start_tick = 0;

#define DMA_MAX_TRANSFER_SIZE 65535UL

void wait_buffer_unlocked() {
    while (s_lock_buffer) {
    }
}

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

    // Register refresh end callback
    HAL_DSI_RegisterCallback(hdsi, HAL_DSI_ENDOF_REFRESH_CB_ID,
                             lcd_end_of_refresh_callback);

    lcd_refresh();

    HAL_LTDC_SetAddress(hltdc, (uint32_t)s_current_frontbuffer, LTDC_LAYER_2);

    // Touchscreen init
    // Give some time after reset for the TS driver to become ready
    HAL_Delay(300);

    if (ft6336g_init() != STATUS_OK) {
        return STATUS_ERROR;
    }

    // Setup DMA for buffer swapping
    __HAL_RCC_DMA2_CLK_ENABLE();
    NVIC_SetPriority(DMA2_Stream0_IRQn, 0);
    NVIC_EnableIRQ(DMA2_Stream0_IRQn);

    hdma.Instance = DMA2_Stream0;
    hdma.Init.Channel = DMA_CHANNEL_0;
    hdma.Init.Direction = DMA_MEMORY_TO_MEMORY;
    hdma.Init.PeriphInc = DMA_PINC_ENABLE;
    hdma.Init.MemInc = DMA_MINC_ENABLE;
    hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma.Init.Mode = DMA_NORMAL;
    hdma.Init.Priority = DMA_PRIORITY_HIGH;
    hdma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    hdma.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_1QUARTERFULL;
    hdma.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma.Init.PeriphBurst = DMA_PBURST_SINGLE;

    if (HAL_DMA_Init(&hdma) != HAL_OK) {
        return STATUS_ERROR;
    }

    HAL_DMA_RegisterCallback(&hdma, HAL_DMA_XFER_CPLT_CB_ID,
                             lcd_dma_tc_callback);

    return STATUS_OK;
}

void lcd_request_refresh() {
    // Set the refresh request flag
    s_refresh_req = true;
}

void lcd_swap_buffers() {
    // First swap the buffer index
    s_current_frontbuffer = 1 - s_current_frontbuffer;

    // Next copy the new frontbuffer (old backbuffer) to the new backbuffer
    // memcpy(BACKBUFFER, FRONTBUFFER, sizeof(s_foreground_buffer_0));
    unsigned long bytes_left =
        sizeof(s_foreground_buffer_0) - s_buffer_swap_bytes;
    unsigned long bytes_to_transfer =
        std::min(bytes_left, DMA_MAX_TRANSFER_SIZE * 4);
    s_lock_buffer = true;
    s_swap_start_tick = HAL_GetTick();
    HAL_DMA_Start_IT(&hdma, (uint32_t)FRONTBUFFER + s_buffer_swap_bytes,
                     (uint32_t)BACKBUFFER + s_buffer_swap_bytes,
                     bytes_to_transfer / 4);
    s_buffer_swap_bytes += bytes_to_transfer;

    // We will set the new frontbuffer after all data have been transferred
}

uint8_t* lcd_get_backbuffer() {
    return BACKBUFFER;
}

uint8_t* lcd_get_frontbuffer() {
    return FRONTBUFFER;
}

void lcd_refresh() {
    HAL_DSI_Refresh(hdsi);
}

void lcd_set_foreground(const uint8_t* fb_address) {
    HAL_DSI_Stop(hdsi);
    __HAL_LTDC_LAYER_DISABLE(hltdc, LTDC_LAYER_2);
    HAL_LTDC_SetAddress(hltdc, (uint32_t)fb_address, LTDC_LAYER_2);
    __HAL_LTDC_LAYER_ENABLE(hltdc, LTDC_LAYER_2);
    HAL_DSI_Start(hdsi);
    lcd_refresh();
}

void lcd_set_background(const uint8_t* fb_address) {
    HAL_DSI_Stop(hdsi);
    __HAL_LTDC_LAYER_DISABLE(hltdc, LTDC_LAYER_1);
    HAL_LTDC_SetAddress(hltdc, (uint32_t)fb_address, LTDC_LAYER_1);
    __HAL_LTDC_LAYER_ENABLE(hltdc, LTDC_LAYER_1);
    HAL_DSI_Start(hdsi);
    lcd_refresh();
}

void lcd_clear_foreground() {
    wait_buffer_unlocked();
    memset(BACKBUFFER, 0x00, sizeof(s_foreground_buffer_0));
}

void lcd_clear_area(unsigned int xl, unsigned int xr, unsigned int yb,
                    unsigned int yt) {
    for (unsigned int xi = xl; xi <= xr; xi++) {
        for (unsigned int yi = yb; yi <= yt; yi++) {
            wait_buffer_unlocked();
            BACKBUFFER[yi + xi * LCD_HEIGHT] = 0x00;
        }
    }
}

void lcd_draw_rectangle(unsigned int x, unsigned int y, unsigned int w,
                        unsigned int h, uint8_t color) {
    for (unsigned int xi = x; xi < x + w; xi++) {
        for (unsigned int yi = y; yi < y + h; yi++) {
            wait_buffer_unlocked();
            BACKBUFFER[yi + xi * LCD_HEIGHT] = color;
        }
    }
}

void lcd_draw_circle(unsigned int x, unsigned int y, unsigned int r,
                     uint8_t color) {
    for (unsigned int xi = x - r; xi < x + r; xi++) {
        for (unsigned int yi = y - r; yi < y + r; yi++) {
            int dx = (int)xi - (int)x;
            int dy = (int)yi - (int)y;

            if ((unsigned int)(dx * dx + dy * dy) < r * r) {
                wait_buffer_unlocked();
                BACKBUFFER[yi + xi * LCD_HEIGHT] = color;
            }
        }
    }
}

void lcd_copy_background_to_foreground(const uint32_t* fb_address) {
    wait_buffer_unlocked();
    if (fb_address != NULL) {
        memcpy(BACKBUFFER, fb_address, sizeof(s_foreground_buffer_0));
    } else {
        memcpy(BACKBUFFER, (uint32_t*)hltdc->LayerCfg[0].FBStartAdress,
               sizeof(s_foreground_buffer_0));
    }
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
    while ((hltdc->Instance->CPSR & 0xFFFF) >= LTDC_VSYNC &&
           (hltdc->Instance->CPSR & 0xFFFF) <
               (LTDC_VBP + LTDC_HEIGHT + LTDC_VSYNC - 2)) {
    }
}

void lcd_draw_char(const Font* font, char ch, unsigned start_x,
                   unsigned start_y, unsigned pt_size, uint8_t color,
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
            int dest_y = (start_y + pt_size - y);
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
                wait_buffer_unlocked();
                BACKBUFFER[(start_y + pt_size - y) +
                           (start_x + x) * LCD_HEIGHT] = color;
            }
        }
    }
}

void lcd_draw_text(const Font* font, const char* str, unsigned start_x,
                   unsigned start_y, unsigned pt_size, uint8_t color) {
    unsigned int cur_x = start_x;
    unsigned int advance = 0;
    while (*str != '\0') {
        lcd_draw_char(font, *str, cur_x, start_y, pt_size, color, &advance);
        cur_x += advance;
        str++;
    }
}

void lcd_end_of_refresh_callback(DSI_HandleTypeDef* hdsi) {
    // Frame rate tracking
    static unsigned int last_tick = 0;
    static unsigned int frame_count = 0;
    frame_count++;
    unsigned int current_tick = HAL_GetTick();
    if (frame_count >= 10) {
        unsigned int delta = current_tick - last_tick;
        unsigned int fps = (frame_count * 1000) / delta;
        printf("FPS: %u\n", fps);
        frame_count = 0;
        last_tick = current_tick;
    }

    static unsigned long last_swap_tick = HAL_GetTick();

    // Only swap the buffers if a refresh was requested
    if (s_refresh_req) {
        printf("Time since last buffer swap %lu ms\n",
               HAL_GetTick() - last_swap_tick);
        last_swap_tick = HAL_GetTick();

        lcd_swap_buffers();
        s_refresh_req = false;
    }
}

void lcd_dma_tc_callback(DMA_HandleTypeDef* hdma) {
    // If there are more bytes to transfer, continue
    if (s_buffer_swap_bytes < sizeof(s_foreground_buffer_0)) {
        unsigned long bytes_left =
            sizeof(s_foreground_buffer_0) - s_buffer_swap_bytes;
        unsigned long bytes_to_transfer =
            std::min(bytes_left, DMA_MAX_TRANSFER_SIZE * 4);
        HAL_DMA_Start_IT(hdma, (uint32_t)FRONTBUFFER + s_buffer_swap_bytes,
                         (uint32_t)BACKBUFFER + s_buffer_swap_bytes,
                         bytes_to_transfer / 4);
        s_buffer_swap_bytes += bytes_to_transfer;
        return;
    }

    s_buffer_swap_bytes = 0;
    s_lock_buffer = false;
    printf("Buffer swap completed in %lu ms\n",
           HAL_GetTick() - s_swap_start_tick);

    // Finally update the framebuffer address and refresh the display
    lcd_set_foreground(FRONTBUFFER);
}

extern "C" void DMA2_Stream0_IRQHandler(void) {
    HAL_DMA_IRQHandler(&hdma);
}