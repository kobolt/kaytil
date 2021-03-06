#ifndef _MEM_H
#define _MEM_H

#include <stdint.h>
#include <stdio.h>

typedef struct mem_s {
  uint8_t ram[UINT16_MAX + 1];
} mem_t;

void mem_init(mem_t *mem);
uint8_t mem_read(mem_t *mem, uint16_t address);
void mem_read_area(mem_t *mem, uint16_t address, uint8_t data[], size_t size);
void mem_write(mem_t *mem, uint16_t address, uint8_t value);
void mem_write_area(mem_t *mem, uint16_t address, uint8_t data[], size_t size);
int mem_load_from_file(mem_t *mem, const char *filename, uint16_t address);
void mem_dump(FILE *fh, mem_t *mem, uint16_t start, uint16_t end);

#endif /* _MEM_H */
