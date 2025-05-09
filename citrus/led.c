#include <iodefine.h>
#include "led.h"

#define LED_PIN PORTA.PDR.BIT.B0
#define LED_OUT PORTA.PODR.BIT.B0

void led_setup(void)
{
  /* Set LED to output mode. */
  LED_PIN = 1;

  /* Turn off LED. */
  LED_OUT = 0;
}

void led_command(LED_COMMAND cmd)
{
  switch (cmd) {
  case LED_OFF:
    LED_OUT = 0;
    break;

  case LED_ON:
    LED_OUT = 1;
    break;

  case LED_TOGGLE:
    LED_OUT ^= 1;
    break;
  }
}

