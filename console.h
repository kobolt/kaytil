#ifndef _CONSOLE_H
#define _CONSOLE_H

#include <stdint.h>
#include <stdbool.h>

uint8_t console_status(void);
uint8_t console_read(void);
void console_write(uint8_t value);

#endif /* _CONSOLE_H */
