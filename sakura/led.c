#include <iodefine.h>
#include "led.h"

#define LED_D1_PIN PORTA.PDR.BIT.B0
#define LED_D2_PIN PORTA.PDR.BIT.B1
#define LED_D3_PIN PORTA.PDR.BIT.B2
#define LED_D4_PIN PORTA.PDR.BIT.B6
#define LED_D1 PORTA.PODR.BIT.B0
#define LED_D2 PORTA.PODR.BIT.B1
#define LED_D3 PORTA.PODR.BIT.B2
#define LED_D4 PORTA.PODR.BIT.B6

void led_setup(void)
{
  /* Set all LED related pins to output mode. */
  LED_D1_PIN = 1;
  LED_D2_PIN = 1;
  LED_D3_PIN = 1;
  LED_D4_PIN = 1;

  /* Turn off all LEDs. */
  LED_D1 = 0;
  LED_D2 = 0;
  LED_D3 = 0;
  LED_D4 = 0;
}

void led_d1_command(LED_COMMAND cmd)
{
  switch (cmd) {
  case LED_OFF:
    LED_D1 = 0;
    break;

  case LED_ON:
    LED_D1 = 1;
    break;

  case LED_TOGGLE:
    LED_D1 ^= 1;
    break;
  }
}

void led_d2_command(LED_COMMAND cmd)
{
  switch (cmd) {
  case LED_OFF:
    LED_D2 = 0;
    break;

  case LED_ON:
    LED_D2 = 1;
    break;

  case LED_TOGGLE:
    LED_D2 ^= 1;
    break;
  }
}

void led_d3_command(LED_COMMAND cmd)
{
  switch (cmd) {
  case LED_OFF:
    LED_D3 = 0;
    break;

  case LED_ON:
    LED_D3 = 1;
    break;

  case LED_TOGGLE:
    LED_D3 ^= 1;
    break;
  }
}

void led_d4_command(LED_COMMAND cmd)
{
  switch (cmd) {
  case LED_OFF:
    LED_D4 = 0;
    break;

  case LED_ON:
    LED_D4 = 1;
    break;

  case LED_TOGGLE:
    LED_D4 ^= 1;
    break;
  }
}

