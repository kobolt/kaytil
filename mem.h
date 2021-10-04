#ifndef _MEM_H
#define _MEM_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct mem_s {
  uint8_t ram[UINT16_MAX + 1];
} mem_t;

void mem_init(mem_t *mem);
void mem_rom_select(mem_t *mem, bool select);
uint8_t mem_read(mem_t *mem, uint16_t address);
void mem_read_area(mem_t *mem, uint16_t address, uint8_t data[], size_t size);
void mem_write(mem_t *mem, uint16_t address, uint8_t value);
void mem_write_area(mem_t *mem, uint16_t address, uint8_t data[], size_t size);

#endif /* _MEM_H */
