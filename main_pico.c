#include <stdlib.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "z80.h"
#include "mem.h"
#include "disk.h"
#include "console.h"
#include "panic.h"

extern uint8_t _binary_cbios_bin_start[];
extern uint8_t _binary_cpm22_bin_start[];
extern uint8_t _binary_cbios_bin_end[];
extern uint8_t _binary_cpm22_bin_end[];



static z80_t z80;
static mem_t mem;



int main(void)
{
  /* Setup LED for disk activity. */
#ifdef PICO_DEFAULT_LED_PIN
  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
  gpio_put(PICO_DEFAULT_LED_PIN, 0);
#endif /* PICO_DEFAULT_LED_PIN */

  console_init();
  z80_init(&z80);
  mem_init(&mem);

  /* Load CP/M 2.2 and CBIOS. */
  mem_write_area(&mem, 0xE400, _binary_cpm22_bin_start,
    _binary_cpm22_bin_end - _binary_cpm22_bin_start);
  mem_write_area(&mem, 0xFA00, _binary_cbios_bin_start,
    _binary_cbios_bin_end - _binary_cbios_bin_start);
  /* Need to set the PC directly to the BIOS,
     since this one will initialize the data area in the zero page. */
  z80.pc = 0xFA00;

  while (1) {
    z80_execute(&z80, &mem);
  }

  return 0;
}



