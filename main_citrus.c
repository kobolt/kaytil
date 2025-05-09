#include <stdlib.h>
#include <stdint.h>
#include "z80.h"
#include "mem.h"
#include "disk.h"
#include "console.h"
#include "panic.h"
#include "led.h"
#include "timer.h"
#include "uart.h"

extern uint8_t binary_cbios_bin_start[];
extern uint8_t binary_cpm22_bin_start[];
extern uint8_t binary_cbios_bin_end[];
extern uint8_t binary_cpm22_bin_end[];



static z80_t z80;
static mem_t mem;



void panic(const char *format, ...)
{
  (void)format;

  /* Light the LED in the event of a panic. */
  led_command(LED_ON);

  while (1) {
    asm("wait");
  }
}



int main(void)
{
  /* GR-CITRUS specific initalization. */
  asm("clrpsw i");
  led_setup();
  timer_setup();
  uart0_setup();
  asm("setpsw i");

  z80_init(&z80);
  mem_init(&mem);

  /* Load CP/M 2.2 and CBIOS. */
  mem_write_area(&mem, 0xE400, binary_cpm22_bin_start,
    binary_cpm22_bin_end - binary_cpm22_bin_start);
  mem_write_area(&mem, 0xFA00, binary_cbios_bin_start,
    binary_cbios_bin_end - binary_cbios_bin_start);
  /* Need to set the PC directly to the BIOS,
     since this one will initialize the data area in the zero page. */
  z80.pc = 0xFA00;

  while (1) {
    z80_execute(&z80, &mem);
  }

  return 0;
}



