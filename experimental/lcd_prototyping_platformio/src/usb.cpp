/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "tusb.h"
#include "status.h"
#include "stm32f4xx.h"

extern "C" {
void OTG_FS_IRQHandler(void);
void tud_mount_cb(void);
void tud_umount_cb(void);
void tud_cdc_line_state_cb(uint8_t instance, bool dtr, bool rts);
}

static void cdc_task(void);

/*------------- MAIN -------------*/
Status usb_init(void) {
    // Low-level init
    __HAL_RCC_USB_OTG_FS_CLK_ENABLE();
    NVIC_SetPriority(OTG_FS_IRQn, 0);
    NVIC_EnableIRQ(OTG_FS_IRQn);

    USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_VBDEN;

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // init device stack on configured roothub port
    tusb_rhport_init_t dev_init = {.role = TUSB_ROLE_DEVICE,
                                   .speed = TUSB_SPEED_FULL};

    if (tusb_init(BOARD_TUD_RHPORT, &dev_init) != true) {
        TRACE_PRINTF("Failed to initialize USB\n");
        return STATUS_ERROR;
    }

    TRACE_PRINTF("USB initialized\n");

    return STATUS_OK;
}

void usb_task(void) {
    tud_task();
    cdc_task();
}

// echo to either Serial0 or Serial1
// with Serial0 as all lower case, Serial1 as all upper case
static void echo_serial_port(uint8_t buf[], uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        tud_cdc_n_write_char(0, buf[i]);
    }
    tud_cdc_n_write_flush(0);
}

// Invoked when device is mounted
void tud_mount_cb(void) {
    TRACE_PRINTF("USB mounted\n");
}

// Invoked when device is unmounted
void tud_umount_cb(void) {
    TRACE_PRINTF("USB unmounted\n");
}

//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
static void cdc_task(void) {
    if (tud_cdc_n_available(0)) {
        uint8_t buf[64];
        uint32_t count = tud_cdc_n_read(0, buf, sizeof(buf));

        // echo back to both serial ports
        echo_serial_port(buf, count);
    }
}

// Invoked when cdc when line state changed e.g connected/disconnected
// Use to reset to DFU when disconnect with 1200 bps
void tud_cdc_line_state_cb(uint8_t instance, bool dtr, bool rts) {
    TRACE_PRINTF("New line state: dtr: %d rts: %d\n", dtr, rts);
}

void OTG_FS_IRQHandler(void) {
    tud_int_handler(0);
}