#ifndef __USBCON_H__
#define __USBCON_H__

int usbcon_read(char* buf, int length);
int usbcon_write(char* buf);
void usbcon_init();

#endif
