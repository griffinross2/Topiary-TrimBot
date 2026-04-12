#ifndef USB_H
#define USB_H

#include "status.h"

Status usb_init();
void usb_task();

unsigned int usb_available_write();
unsigned int usb_send(const char* data, unsigned int len);
unsigned int usb_available();
unsigned int usb_receive(char* buf, unsigned int len);

#endif  // USB_H