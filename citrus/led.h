#ifndef _LED_H
#define _LED_H

typedef enum {
  LED_OFF    = 0,
  LED_ON     = 1,
  LED_TOGGLE = 2,
} LED_COMMAND;

void led_setup(void);
void led_command(LED_COMMAND cmd);

#endif /* _LED_H */
