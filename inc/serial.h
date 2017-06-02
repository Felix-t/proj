#ifndef SERIAL_H
#define SERIAL_H

#include "headers.h"

uint8_t set_interface_attribs (int fd, int speed, int parity);
uint8_t set_blocking (int fd, int should_block);


#endif
