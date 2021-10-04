#include <stdlib.h>
#include <stdint.h>
#include "z80.h"
#include "mem.h"
#include "disk.h"
#include "console.h"
#include "panic.h"
#include "fat16.h"
#include "led.h"
#include "timer.h"
#include "uart.h"
#include "sdcard.h"

extern unsigned char cbios_data[];
extern unsigned char cpm22_data[];
extern unsigned int cbios_size;
extern unsigned int cpm22_size;



static z80_t z80;
static mem_t mem;



void panic(const char *format, ...)
{
  (void)format;

  /* Light all LEDs in the event of a panic. */
  led_d1_command(LED_ON);
  led_d2_command(LED_ON);
  led_d3_command(LED_ON);
  led_d4_command(LED_ON);

  while (1) {
    asm("wait");
  }
}



int main(void)
{
  /* GR-SAKURA specific initalization. */
  asm("clrpsw i");
  led_setup();
  timer_setup();
  uart0_setup();
  sdcard_setup();
  asm("setpsw i");

  fat16_cache_clear();
  z80_init(&z80);
  mem_init(&mem);

  /* Load CP/M 2.2 and CBIOS. */
  mem_write_area(&mem, 0xE400, cpm22_data, cpm22_size);
  mem_write_area(&mem, 0xFA00, cbios_data, cbios_size);
  /* Need to set the PC directly to the BIOS,
     since this one will initialize the data area in the zero page. */
  z80.pc = 0xFA00;

  while (1) {
    z80_execute(&z80, &mem);
  }

  return 0;
}



