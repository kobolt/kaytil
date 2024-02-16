#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <curses.h>

#include "panic.h"



static void console_exit(void)
{
  endwin();
}



void console_init(void)
{
  initscr();
  atexit(console_exit);
  noecho();
  keypad(stdscr, TRUE);
  timeout(0); /* Non-blocking mode. */
}



uint8_t console_status(void)
{
  int ch;
  ch = getch();
  if (ch == ERR) {
    return 0x00;
  } else {
    ungetch(ch);
    return 0xFF;
  }
}



uint8_t console_read(void)
{
  int ch;

  timeout(-1);
  do {
    ch = getch(); /* Blocking read here. */
  } while (ch == ERR);
  timeout(0);

  /* ADM-3A emulation of key presses. */
  switch(ch) {
  case KEY_RIGHT:
    return 0x0C; /* FF */
  case KEY_LEFT:
    return 0x08; /* BS */
  case KEY_DOWN:
    return 0x0A; /* LF */
  case KEY_UP:
    return 0x0B; /* VT */
  case KEY_BACKSPACE:
    return 0x08; /* BS */
  case KEY_ENTER:
    return 0x0D; /* CR */
  case 0x0A: /* Convert LF */
    return 0x0D; /* CR */
  default:
    break;
  }

  if (ch >= 0 && ch <= 0xFF) {
    return (uint8_t)ch;
  } else {
    return 0;
  }
}



void console_write(uint8_t value)
{
  static int escape = 0;
  static uint8_t row = 0;
  int y, x;

  /* ADM-3A emulation of escape codes. */
  if (escape == 2) {
    row = value;
    escape++;
    return;

  } else if (escape == 3) {
    move(row - 32, value - 32);
    escape++;
    refresh();
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
      addch(value);
      refresh();
      return;
    }
  }

  /* Check potential non-printable characters. */
  switch (value) {
  case 0x07: /* Bell */
    flash();
    break;

  case 0x08: /* Backspace */
    getyx(stdscr, y, x);
    move(y, x - 1);
    break;

  case 0x0A: /* Line Feed */
    getyx(stdscr, y, x);
    move(y + 1, 0);
    break;

  case 0x0B: /* Upline */
    getyx(stdscr, y, x);
    move(y - 1, x);
    break;

  case 0x0C: /* Forward Space */
    getyx(stdscr, y, x);
    move(y, x + 1);
    break;

  case 0x0D: /* Return */
    getyx(stdscr, y, x);
    move(y, 0);
    break;

  case 0x1B: /* Escape */
    if (escape == 0) {
      escape++;
    } else {
      escape = 0;
    }
    break;

  case 0x1A: /* Clear Screen */
    erase();
    /* Fallthrough! */
  case 0x1E: /* Home Cursor */
    move(0, 0);
    break;

  case 0x3D:
    if (escape == 1) {
      escape++;
    } else {
      addch('=');
      escape = 0;
    }
    break;

  case 0xA4: /* Copyright Symbol */
    addch('c');
    break;

  default:
    /* Passthrough all other characters. */
    addch(value);
    break;
  }

  refresh();
}



