#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "panic.h"
#include "uart.h"



uint8_t console_status(void)
{
  return (uart0_pending() == 0) ? 0x00 : 0xFF;
}



uint8_t console_read(void)
{
  uint8_t value;
  
  /* Wait until there is an actual character available. */
  while ((value = uart0_recv()) == '\0') {
    asm("wait");
  }

  switch (value) {
  case 0x0A: /* Convert LF to CR */
    return 0x0D;

  case 0x7F: /* Convert DEL to BS */
    return 0x08;

  case 0x1B: /* Escape */
    if (uart0_recv() == '[') {
      value = uart0_recv();
      switch (value) {
      case 'A': return 0x0B; /* Cursor Up */
      case 'B': return 0x0A; /* Cursor Down */
      case 'C': return 0x0C; /* Cursor Forward/Right */
      case 'D': return 0x08; /* Cursor Back/Left */
      default:
        panic("Unhandled console read escape code: 0x%02x\n", value);
        break;
      }
    }
    break;

  default:
    break;
  }

  return value;
}



void console_write(uint8_t value)
{
  static int escape = 0;
  static uint8_t row = 0;
  char single[2];

  /* ADM-3A emulation of escape codes. */
  if (escape == 2) {
    row = value;
    escape++;
    return;

  } else if (escape == 3) {
    /* ANSI - Cursor Position */
    /* Lite implementation of: printf("\e[%d;%dH", row - 31, value - 31); */

    single[1] = '\0';
    uart0_send("\e[");

    if ((row - 31) >= 10) {
      single[0] = ((row - 31) / 10) + 0x30;
      uart0_send(single);
      single[0] = ((row - 31) % 10) + 0x30;
      uart0_send(single);
    } else {
      single[0] = (row - 31) + 0x30;
      uart0_send(single);
    }
    uart0_send(";");

    if ((value - 31) >= 10) {
      single[0] = ((value - 31) / 10) + 0x30;
      uart0_send(single);
      single[0] = ((value - 31) % 10) + 0x30;
      uart0_send(single);
    } else {
      single[0] = (value - 31) + 0x30;
      uart0_send(single);
    }

    uart0_send("H");
    escape++;
    return;

  } else if (escape == 4) {
    if (value == 0x3D) {
      /* Repeated escape code. */
      escape = 2;
      return;

    } else {
      escape = 0;
    }
  }

  /* Pass through regular ASCII characters. */
  if (value >= 0x20 && value < 0x7F) {
    if (escape == 0) {
      single[0] = value;
      single[1] = '\0';
      uart0_send(single);
      return;
    }
  }

  /* Check potential non-printable characters. */
  switch (value) {
  case 0x0B:
    /* ANSI - Cursor Up */
    uart0_send("\e[A");
    break;

  case 0x0C:
    /* ANSI - Cursor Right */
    uart0_send("\e[C");
    break;

  case 0x1B:
    if (escape == 0) {
      escape++;
    } else {
      escape = 0;
    }
    break;

  case 0x1A:
    /* ANSI - Erase in Display */
    uart0_send("\e[2J");
    /* Fallthrough! */
  case 0x1E:
    /* ANSI - Cursor Home */
    uart0_send("\e[H");
    break;

  case 0x3D:
    if (escape == 1) {
      escape++;
    } else {
      single[0] = '=';
      single[1] = '\0';
      uart0_send(single);
      escape = 0;
    }
    break;

  case 0xA4:
    single[0] = '©';
    single[1] = '\0';
    uart0_send(single);
    break;

  default:
    /* Passthrough all other characters. */
    single[0] = value;
    single[1] = '\0';
    uart0_send(single);
    break;
  }
}



