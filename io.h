#ifndef _IO_H
#define _IO_H

#include <stdint.h>
#include "mem.h"

uint8_t io_read(uint8_t port, uint8_t upper_address);
void io_write(uint8_t port, uint8_t upper_address, uint8_t value, mem_t *mem);

#endif /* _IO_H */
