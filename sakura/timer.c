#include <iodefine.h>
#include "timer.h"

static volatile unsigned char timer_left = 0;

__attribute__((interrupt))
void cmt0_cmi0_handler(void)
{
  if (timer_left > 0) {
    timer_left--;
  }
}

void timer_setup(void)
{
  struct st_cmt0_cmcr cmcr;

  CMT.CMSTR0.BIT.STR0 = 0; /* Stop counting. */

  /* Trigging every ~10ms
   * CMCOR / (PCLK / CMCR.CKS) = ms
   * 937 / (48000000 / 512) = ~0.01
   */
  CMT0.CMCNT = 0;   /* Counting starts at 0... */
  CMT0.CMCOR = 937; /* ...and ends at 937 */

  IPR(CMT0, CMI0) = 1; /* Interrupt Priority 1. */
  IEN(CMT0, CMI0) = 1; /* Interrupt Enable. */
  IR(CMT0, CMI0)  = 0; /* Clear Interrupt Request flag. */

  cmcr.WORD     = 0;    /* Zero out all values. */
  cmcr.BIT.CKS  = 0b11; /* Clock: PCLK/512 */
  cmcr.BIT.CMIE = 1;    /* Compare Match Interrupt Enable. */
  cmcr.BIT.b7   = 1;    /* Set the b7 reserved bit. */
  CMT0.CMCR.WORD = cmcr.WORD;

  CMT.CMSTR0.BIT.STR0 = 1; /* Start counting. */
}

unsigned char timer_read(void)
{
  return timer_left;
}

void timer_set(unsigned char countdown)
{
  timer_left = countdown;
}

