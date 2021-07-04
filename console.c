#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include "panic.h"



static void console_exit_handler(void)
{
  /* Restore canonical mode and echo. */
  struct termios ts;
  tcgetattr(STDIN_FILENO, &ts);
  ts.c_lflag |= ICANON | ECHO;
  tcsetattr(STDIN_FILENO, TCSANOW, &ts);
}



void console_init(void)
{
  atexit(console_exit_handler);

  /* Turn off canonical mode and echo. */
  struct termios ts;
  tcgetattr(STDIN_FILENO, &ts);
  ts.c_lflag &= ~ICANON & ~ECHO;
  tcsetattr(STDIN_FILENO, TCSANOW, &ts);

  /* Make stdout unbuffered. */
  setvbuf(stdout, NULL, _IONBF, 0);
}



uint8_t console_status(void)
{
  int result;
  struct pollfd fds[1];

  fds[0].fd = fileno(stdin);
  fds[0].events = POLLIN;
  result = poll(fds, 1, 0);
  if (result == -1) {
    if (errno == EINTR) {
      return 0x00; /* Handle this as nothing available. */
    } else {
      panic("poll() failed with errno: %d\n", errno);
    }
  }
  return (result == 0) ? 0x00 : 0xFF;
}



uint8_t console_read(void)
{
  uint8_t value = fgetc(stdin);

  switch (value) {
  case 0x0A: /* Convert LF to CR */
    return 0x0D;

  case 0x7F: /* Convert DEL to BS */
    return 0x08;

  case 0x1B: /* Escape */
    if (fgetc(stdin) == '[') {
      value = fgetc(stdin);
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

  /* ADM-3A emulation of escape codes. */
  if (escape == 2) {
    row = value;
    escape++;
    return;

  } else if (escape == 3) {
    /* ANSI - Cursor Position */
    fprintf(stdout, "\e[%d;%dH", row - 31, value - 31);
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
      fputc(value, stdout);
      return;
    }
  }

  /* Check potential non-printable characters. */
  switch (value) {
  case 0x0B:
    /* ANSI - Cursor Up */
    fprintf(stdout, "\e[A");
    break;

  case 0x0C:
    /* ANSI - Cursor Right */
    fprintf(stdout, "\e[C");
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
    fprintf(stdout, "\e[2J");
  case 0x1E:
    /* ANSI - Cursor Home */
    fprintf(stdout, "\e[H");
    break;

  case 0x3D:
    if (escape == 1) {
      escape++;
    } else {
      fputc('=', stdout);
      escape = 0;
    }
    break;

  case 0xA4:
    fputc('©', stdout);
    break;

  default:
    /* Passthrough all other characters. */
    fputc(value, stdout);
    break;
  }
}



