#include "ra8875.h"

#include "stm32f0xx_hal.h"
#include <string.h>
#include <stdio.h>

// Alot of this is pulled from the adafruit driver

SPI_HandleTypeDef hspi2 = {
    .Instance = SPI2,
};

void spi_init() {
    __HAL_RCC_SPI2_CLK_ENABLE();

    GPIO_InitTypeDef gpio_init = {
        .Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_HIGH,
        .Alternate = GPIO_AF0_SPI2,
    };

    HAL_GPIO_Init(GPIOB, &gpio_init);

    gpio_init.Pin = GPIO_PIN_12;
    gpio_init.Mode = GPIO_MODE_OUTPUT_PP;

    HAL_GPIO_Init(GPIOB, &gpio_init);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);

    SPI_InitTypeDef spi_init = {
        .Mode = SPI_MODE_MASTER,
        .Direction = SPI_DIRECTION_2LINES,
        .DataSize = SPI_DATASIZE_8BIT,
        .CLKPolarity = SPI_POLARITY_LOW,
        .CLKPhase = SPI_PHASE_1EDGE,
        .NSS = SPI_NSS_SOFT,
        .BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4,
        .FirstBit = SPI_FIRSTBIT_MSB,
        .TIMode = SPI_TIMODE_DISABLE,
        .CRCCalculation = SPI_CRCCALCULATION_DISABLE,
        .CRCPolynomial = 0,
    };
    hspi2.Init = spi_init;

    HAL_SPI_Init(&hspi2);
}

void reg_write(uint8_t reg) {
    uint16_t buf = 0x80 | (reg << 8);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);

    HAL_SPI_Transmit(&hspi2, (uint8_t*)&buf, 2, HAL_MAX_DELAY);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
}

uint8_t status_read() {
    uint16_t tx_buf = 0x00C0;
    uint16_t rx_buf = 0;

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);

    HAL_SPI_TransmitReceive(&hspi2, (uint8_t*)&tx_buf, (uint8_t*)&rx_buf, 2,
                            HAL_MAX_DELAY);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);

    return (rx_buf >> 8) & 0xFF;
}

void data_write(uint8_t data) {
    uint16_t buf = 0x00 | ((uint16_t)data << 8);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);

    HAL_SPI_Transmit(&hspi2, (uint8_t*)&buf, 2, HAL_MAX_DELAY);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
}

void data_write_multiple(const uint8_t* data, unsigned int size) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);

    uint8_t first_byte = 0x00;
    HAL_SPI_Transmit(&hspi2, &first_byte, 1, HAL_MAX_DELAY);
    HAL_SPI_Transmit(&hspi2, const_cast<uint8_t*>(data), size, HAL_MAX_DELAY);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
}

void data_write_multiple(uint8_t val, unsigned int num) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);

    uint8_t first_byte = 0x00;
    HAL_SPI_Transmit(&hspi2, &first_byte, 1, HAL_MAX_DELAY);
    for (unsigned int i = 0; i < num; ++i) {
        HAL_SPI_Transmit(&hspi2, &val, 1, HAL_MAX_DELAY);
    }

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
}

uint8_t data_read() {
    uint16_t tx_buf = 0x0040;
    uint16_t rx_buf = 0;

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);

    HAL_SPI_TransmitReceive(&hspi2, (uint8_t*)&tx_buf, (uint8_t*)&rx_buf, 2,
                            HAL_MAX_DELAY);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);

    return (rx_buf >> 8) & 0xFF;
}

Status ra8875_init() {
    spi_init();

    GPIO_InitTypeDef gpio_init = {
        .Pin = GPIO_PIN_8,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW,
    };

    HAL_GPIO_Init(GPIOB, &gpio_init);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
    HAL_Delay(100);

    // Check ID
    reg_write(0x00);
    uint8_t id = data_read();
    if (id != 0x75) {
        return STATUS_ERROR;
    }

    // PLL control
    reg_write(RA8875_REG_PLLC1);
    data_write(11);
    HAL_Delay(1);
    reg_write(RA8875_REG_PLLC2);
    data_write(2);
    HAL_Delay(1);

    // System register
    reg_write(RA8875_REG_SYSR);
    data_write(0x00);  // 8bpp, 8bit mcu

    // Display control
    reg_write(RA8875_REG_DPCR);
    data_write(0x80);  // 2 layers

    // Touch panel enable
    reg_write(RA8875_REG_TPCR0);
    data_write(0b10111100);  // Enable touch panel
    reg_write(RA8875_REG_TPCR1);
    data_write(0b00000100);  // 12MHz touch panel clock

    // Interrupt enable
    reg_write(RA8875_REG_INTC1);
    data_write(0x04);  // Enable touch panel interrupt

    // Timings/display settings
    uint16_t width = 800;
    uint16_t height = 480;
    uint8_t pixclk = 0x81;
    uint8_t hsync_start = 32;
    uint8_t hsync_pw = 96;
    uint8_t hsync_finetune = 0;
    uint8_t hsync_nondisp = 26;
    uint8_t vsync_pw = 2;
    uint16_t vsync_nondisp = 32;
    uint16_t vsync_start = 23;
    uint8_t voffset = 0;

    reg_write(RA8875_REG_PCSR);
    data_write(pixclk);
    HAL_Delay(1);

    reg_write(RA8875_REG_HDWR);
    data_write(width / 8 - 1);
    reg_write(RA8875_REG_HNDFTR);
    data_write(hsync_finetune);
    reg_write(RA8875_REG_HNDR);
    data_write((hsync_nondisp - hsync_finetune - 2) / 8);
    reg_write(RA8875_REG_HSTR);
    data_write(hsync_start / 8 - 1);
    reg_write(RA8875_REG_HPWR);
    data_write(hsync_pw / 8 - 1);

    reg_write(RA8875_REG_VDHR0);
    data_write((height - 1 + voffset) & 0xFF);
    reg_write(RA8875_REG_VDHR1);
    data_write(((height - 1 + voffset) >> 8) & 0xFF);
    reg_write(RA8875_REG_VNDR0);
    data_write((vsync_nondisp - 1) & 0xFF);
    reg_write(RA8875_REG_VNDR1);
    data_write((vsync_nondisp >> 8) & 0xFF);
    reg_write(RA8875_REG_VSTR0);
    data_write((vsync_start - 1) & 0xFF);
    reg_write(RA8875_REG_VSTR1);
    data_write((vsync_start >> 8) & 0xFF);
    reg_write(RA8875_REG_VPWR);
    data_write(vsync_pw - 1);

    reg_write(RA8875_REG_HSAW0);
    data_write(0x00);
    reg_write(RA8875_REG_HSAW1);
    data_write(0x00);
    reg_write(RA8875_REG_HEAW0);
    data_write((width - 1) & 0xFF);
    reg_write(RA8875_REG_HEAW1);
    data_write(((width - 1) >> 8) & 0xFF);

    reg_write(RA8875_REG_VSAW0);
    data_write(voffset);
    reg_write(RA8875_REG_VSAW1);
    data_write(voffset);
    reg_write(RA8875_REG_VEAW0);
    data_write((height - 1 + voffset) & 0xFF);
    reg_write(RA8875_REG_VEAW1);
    data_write(((height - 1 + voffset) >> 8) & 0xFF);

    reg_write(RA8875_REG_MCLR);
    data_write(0x80);

    // Disable PWM0 and set high
    reg_write(0x8A);
    data_write(0x40);

    HAL_Delay(500);

    reg_write(RA8875_REG_PWRR);
    data_write(0x80);  // Display on
    reg_write(RA8875_REG_GPIOX);
    data_write(0x1);  // Enable GPIOX output (DE)

    // Set layer 0 as active and layer 1 for memory writes
    ra8875_set_active_layer(0);
    ra8875_set_memory_write_layer(1);

    // while (1) {
    //     reg_write(RA8875_REG_INTC2);
    //     uint8_t int_status = data_read();
    //     if (int_status & 0x04) {
    //         reg_write(0x72);
    //         uint8_t tp_x_high = data_read();
    //         reg_write(0x73);
    //         uint8_t tp_y_high = data_read();
    //         reg_write(0x74);
    //         uint8_t tp_status = data_read();

    //         unsigned int tp_x = (tp_x_high << 2) | (tp_status & 0x03);
    //         unsigned int tp_y = (tp_y_high << 2) | ((tp_status >> 2) & 0x03);

    //         printf("Touch at (%u, %u)\n", tp_x, tp_y);

    //         // Clear interrupt
    //         reg_write(RA8875_REG_INTC2);
    //         data_write(0x04);
    //     }
    // }

    return STATUS_OK;
}

void ra8875_set_active_layer(unsigned int layer) {
    reg_write(RA8875_REG_LTPR0);
    data_write(layer & 0x1);
}

void ra8875_set_memory_write_layer(unsigned int layer) {
    reg_write(RA8875_REG_MWCR1);
    uint8_t mwcr1 = data_read();
    mwcr1 &= ~0x1;
    mwcr1 |= (layer & 0x1);
    reg_write(RA8875_REG_MWCR1);
    data_write(mwcr1);
}

void ra8875_set_memory_write_position(unsigned int x, unsigned int y) {
    reg_write(RA8875_REG_CURH0);
    data_write(x & 0xFF);
    reg_write(RA8875_REG_CURH1);
    data_write((x >> 8) & 0x3);
    reg_write(RA8875_REG_CURV0);
    data_write(y & 0xFF);
    reg_write(RA8875_REG_CURV1);
    data_write((y >> 8) & 0x1);
}

void ra8875_memory_write_byte(uint8_t data) {
    reg_write(RA8875_REG_MRWC);
    data_write(data);
}

void ra8875_memory_write_multiple(const uint8_t* data, unsigned int size) {
    reg_write(RA8875_REG_MRWC);
    data_write_multiple(data, size);
}

void ra8875_memory_write_multiple(uint8_t value, unsigned int num) {
    reg_write(RA8875_REG_MRWC);
    data_write_multiple(value, num);
}

void ra8875_set_pixel(unsigned int x, unsigned int y, uint8_t color) {
    ra8875_set_memory_write_position(x, y);
    ra8875_memory_write_byte(color);
}

void ra8875_draw_rectangle(unsigned int x, unsigned int y, unsigned int w,
                           unsigned int h, uint8_t color) {
    reg_write(0x91);
    data_write(x & 0xFF);
    reg_write(0x92);
    data_write((x >> 8) & 0x3);
    reg_write(0x93);
    data_write(y & 0xFF);
    reg_write(0x94);
    data_write((y >> 8) & 0x1);

    reg_write(0x95);
    data_write((x + w - 1) & 0xFF);
    reg_write(0x96);
    data_write(((x + w - 1) >> 8) & 0x3);
    reg_write(0x97);
    data_write((y + h - 1) & 0xFF);
    reg_write(0x98);
    data_write(((y + h - 1) >> 8) & 0x1);

    reg_write(0x63);
    data_write(color >> 5);
    reg_write(0x64);
    data_write((color >> 2) & 0x7);
    reg_write(0x65);
    data_write(color & 0x3);

    reg_write(0x90);
    data_write(0xB0);  // Start rectangle fill

    // Wait for completion
    while (1) {
        reg_write(0x90);
        uint8_t status = data_read();
        if ((status & 0x80) == 0) {
            break;
        }
        for (int i = 0; i < 50; ++i) {
            __NOP();
        }
    }
}

void ra8875_clear_memory() {
    reg_write(RA8875_REG_MCLR);
    data_write(0xC0);
}