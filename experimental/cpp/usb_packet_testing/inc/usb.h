#pragma once

#define VID_TRIMBOT 0x4842
#define PID_TRIMBOT 0x4001

#define USB_TX_BUFFER_SIZE 512
#define USB_RX_BUFFER_SIZE 512

#define EPNUM_CDC_0_NOTIF 0x81
#define EPNUM_CDC_0_OUT 0x02
#define EPNUM_CDC_0_IN 0x82

int usb_init();
int usb_deinit();

int usb_connect(int vendor_id = VID_TRIMBOT, int product_id = PID_TRIMBOT);
int usb_disconnect();

int usb_send(const char *data, int len);
int usb_available();
int usb_receive(char *buf, int len);

void usb_task();