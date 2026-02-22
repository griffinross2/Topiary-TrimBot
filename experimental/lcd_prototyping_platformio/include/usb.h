#ifndef USB_H
#define USB_H

#include "status.h"

Status usb_init();
void usb_task();

int usb_available_write();
int usb_send(const char* data, int len);
int usb_available();
int usb_receive(char* buf, int len);

#endif  // USB_H