#include <iodefine.h>
#include "uart.h"

static volatile char uart0_recv_byte = '\0';
static volatile char *uart0_send_byte;
static volatile char uart0_send_done = 0;

__attribute__((interrupt))
void sci0_rxi0_handler(void)
{
  uart0_recv_byte = SCI0.RDR;
}

__attribute__((interrupt))
void sci0_txi0_handler(void)
{
  if (*uart0_send_byte != '\0') {
    SCI0.TDR = *uart0_send_byte;
    uart0_send_byte++;
  } else {
    uart0_send_done = 1;
  }
}

void uart0_setup(void)
{
  struct st_sci0_ssr ssr;

  PORT2.PDR.BIT.B0  = 1; /* Set the TXD0 pin as output. */
  PORT2.PDR.BIT.B1  = 0; /* Set the RXD0 pin as input. */

  PORT2.PODR.BIT.B0 = 1; /* Set the TXD0 pin high. */

  SYSTEM.PRCR.WORD = 0xa502; /* Enable writing to MSTP registers. */
  MSTP_SCI0        = 0;      /* Disable module-stop state for SCI0. */
  SYSTEM.PRCR.WORD = 0xa500; /* Disable writing to MSTP registers. */

  SCI0.SCR.BYTE = 0;  /* Use internal clock, on-chip baud rate generator. */
  SCI0.SMR.BYTE = 0;  /* 8 data bits, 1 stop bit, no parity. */

  SCI0.SEMR.BIT.NFEN = 1; /* Noise filter for RXD0 pin. */

  /* Baud rate calculation:
   * PCLK / (32 * bps) = BRR
   * 48000000 / (32 * 9600) = 155
   * 48000000 / (32 * 115200) = ~12
   */
  SCI0.BRR = 12;

  ssr.BYTE      = SCI0.SSR.BYTE;
  ssr.BIT.ORER  = 0; /* Clear Overrun Error Flag. */
  ssr.BIT.FER   = 0; /* Clear Framing Error Flag. */
  ssr.BIT.PER   = 0; /* Clear Parity Error Flag. */
  ssr.BIT.b7_6  = 0b11;
  SCI0.SSR.BYTE = ssr.BYTE;

  IPR(SCI0, RXI0) = 2; /* Interrupt Priority 2. */
  IEN(SCI0, RXI0) = 1; /* Interrupt Enable. */
  IR(SCI0, RXI0)  = 0; /* Clear Interrupt Request flag. */

  IPR(SCI0, TXI0) = 2; /* Interrupt Priority 2. */
  IEN(SCI0, TXI0) = 1; /* Interrupt Enable. */
  IR(SCI0, TXI0)  = 0; /* Clear Interrupt Request flag. */

  SCI0.SCR.BIT.TIE = 1; /* Set Transmit Interrupt Enable. */
  SCI0.SCR.BIT.RIE = 1; /* Set Receive Interrupt Enable. */
  SCI0.SCR.BIT.TE  = 1; /* Set Transmit Enable. */
  SCI0.SCR.BIT.RE  = 1; /* Set Receive Enable. */

  MPC.PWPR.BIT.B0WI  = 0; /* Enable the PFSWE modification. */
  MPC.PWPR.BIT.PFSWE = 1; /* Disable the PFS register protect. */
  MPC.P20PFS.BYTE    = 0b00001010; /* Use Port 20 as TXD0. */
  MPC.P21PFS.BYTE    = 0b00001010; /* Use Port 21 as RXD0. */
  MPC.PWPR.BIT.PFSWE = 0; /* Enable the PFS register protect. */
  MPC.PWPR.BIT.B0WI  = 1; /* Disable the PFSWE modification. */

  PORT2.PMR.BIT.B0 = 1; /* Use TXD0 pin for peripheral functions. */
  PORT2.PMR.BIT.B1 = 1; /* Use RXD0 pin for peripheral functions. */
}

void uart0_send(char *s)
{
  if (*s == '\0')
    return;

  uart0_send_byte = s;
  uart0_send_done = 0;

  /* Generate initial interrupt... */
  IR(SCI0, TXI0) = 1;

  /* ...and wait for transmission to complete. */
  while (uart0_send_done == 0) {
    asm("nop");
  }
}

char uart0_recv(void)
{
  char c;

  if (uart0_recv_byte != '\0') {
    c = uart0_recv_byte;
    uart0_recv_byte = '\0';
  } else {
    c = '\0';
  }

  return c;
}

int uart0_pending(void)
{
  return (uart0_recv_byte == '\0') ? 0 : 1;
}

