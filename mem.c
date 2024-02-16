#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "mem.h"



void mem_init(mem_t *mem)
{
  int i;
  for (i = 0; i <= UINT16_MAX; i++) {
    mem->ram[i] = 0x0;
  }
}



uint8_t mem_read(mem_t *mem, uint16_t address)
{
  return mem->ram[address];
}



void mem_read_area(mem_t *mem, uint16_t address, uint8_t data[], size_t size)
{
  size_t i;
  for (i = 0; i < size; i++) {
    data[i] = mem->ram[address + i];
  }
}



void mem_write(mem_t *mem, uint16_t address, uint8_t value)
{
  mem->ram[address] = value;
}



void mem_write_area(mem_t *mem, uint16_t address, uint8_t data[], size_t size)
{
  size_t i;
  for (i = 0; i < size; i++) {
    mem->ram[address + i] = data[i];
  }
}



int mem_load_from_file(mem_t *mem, const char *filename, uint16_t address)
{
  FILE *fh;
  int c;

  fh = fopen(filename, "rb");
  if (fh == NULL) {
    return -1;
  }

  while ((c = fgetc(fh)) != EOF) {
    mem->ram[address] = c;
    address++; /* Just overflow... */
  }

  fclose(fh);
  return 0;
}



void mem_dump(FILE *fh, mem_t *mem, uint16_t start, uint16_t end)
{
  int i;
  for (i = start; i <= end; i++) {
    if (i % 16 == 0) {
      fprintf(fh, "%04x   ", i);
    }
    fprintf(fh, "%02x ", mem->ram[i]);
    if (i % 16 == 15) {
      fprintf(fh, "\n");
    }
  }
}



