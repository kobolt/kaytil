#include <stdlib.h>
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
  for (unsigned int i = 0; i < size; i++) {
    data[i] = mem->ram[address + i];
  }
}



void mem_write(mem_t *mem, uint16_t address, uint8_t value)
{
  mem->ram[address] = value;
}



void mem_write_area(mem_t *mem, uint16_t address, uint8_t data[], size_t size)
{
  for (unsigned int i = 0; i < size; i++) {
    mem->ram[address + i] = data[i];
  }
}



