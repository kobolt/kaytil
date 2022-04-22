#include <iodefine.h>
#include "sdcard.h"

/* NOTE: Timeouts increased by a factor of 100 from the default! */
#define COMMAND_FINISH_CYCLE_MAX 1000
#define CMD0_FINISH_CYCLE_MAX    1000
#define ACMD41_FINISH_CYCLE_MAX  1000
#define CMD17_FINISH_CYCLE_MAX   3200

static int sdcard_initialized = 0;

static unsigned char spi_transfer(unsigned char data)
{
  RSPI0.SPSR.BYTE = 0b10100000; /* Clear error bits. */

  /* Send. */
  RSPI0.SPDR.LONG = data;
  while (RSPI0.SPSR.BYTE & 0x2) { /* Wait for IDLNF to become 0. */
    asm("nop");
  }

  /* Receive. */
  data = RSPI0.SPDR.LONG & 0xFF;
  while (RSPI0.SPSR.BYTE & 0x2) { /* Wait for IDLNF to become 0. */
    asm("nop");
  }

  return data;
}

static void spi_dummy_bytes(unsigned long amount)
{
  while (amount > 0) {
    (void)spi_transfer(0xFF);
    amount--;
  }
}

static char spi_sdcard_command(char cmd, long arg)
{
  int offset;
  char response;
  volatile int cycle;

  PORTC.PODR.BIT.B0 = 0; /* Reset CS. */

  /* Send 1 command byte. */
  spi_transfer(cmd | 0x40); /* Include transmission bit. */

  /* Send 4 argument bytes. */
  for (offset = 24; offset >= 0; offset -= 8) {
    (void)spi_transfer((arg >> offset) & 0xFF);
  }

  /* Send 1 CRC byte. */
  switch (cmd) {
  case 0: /* Hardcoded for CMD0. */
    (void)spi_transfer(0x95);
    break;
  case 8: /* Hardcoded for CMD8 and 0x1AA argument. */
    (void)spi_transfer(0x87);
    break;
  default: /* No real CRC for other commands. */
    (void)spi_transfer(0x00);
    break;
  }

  /* Wait for command to finish, first response byte MSB will be 0. */
  cycle = 0;
  while ((response = spi_transfer(0xFF)) & 0x80) {
    cycle++;
    if (cycle > COMMAND_FINISH_CYCLE_MAX) {
      return 0xFF; /* Timeout! */
    }
  }

  return response; /* Return first byte of response. */
}

void spi_setup(void)
{
  SYSTEM.PRCR.WORD = 0xa502; /* Enable writing to MSTP registers. */
  MSTP_RSPI0       = 0;      /* Disable module-stop state for RSPI 0. */
  SYSTEM.PRCR.WORD = 0xa500; /* Disable writing to MSTP registers. */

  /* Set SPI pin directions. */
  PORTC.PDR.BIT.B0 = 1; /* SD card CS as output. */
  PORTC.PDR.BIT.B5 = 1; /* SPI CLK as output. */
  PORTC.PDR.BIT.B6 = 1; /* SPI MOSI as output. */
  PORTC.PDR.BIT.B7 = 0; /* SPI MISO as input. */

  PORTC.PCR.BIT.B7 = 1; /* Activate internal pull-up for MISO input. */

  /* Activate SPI in MPC. */
  MPC.PWPR.BIT.B0WI  = 0; /* Enable the PFSWE modification. */
  MPC.PWPR.BIT.PFSWE = 1; /* Disable the PFS register protect. */
  MPC.PC5PFS.BIT.PSEL = 0xd; /* Use Port C5 as SPI CLK */
  MPC.PC6PFS.BIT.PSEL = 0xd; /* Use Port C6 as SPI MOSI */
  MPC.PC7PFS.BIT.PSEL = 0xd; /* Use Port C7 as SPI MISO */
  MPC.PWPR.BIT.PFSWE = 0; /* Enable the PFS register protect. */
  MPC.PWPR.BIT.B0WI  = 1; /* Disable the PFSWE modification. */

  /* Set SPI pin modes. */
  PORTC.PMR.BIT.B0 = 0; /* SD card CS as general I/O. */
  PORTC.PMR.BIT.B5 = 1; /* SPI CLK as peripheral. */
  PORTC.PMR.BIT.B6 = 1; /* SPI MOSI as peripheral. */
  PORTC.PMR.BIT.B7 = 1; /* SPI MISO as peripheral. */

  /* Bit rate = PCLK / (2 * (SPBR + 1)) */
  /* 250000 = 48000000 / (2 * (95 + 1)) */
  RSPI0.SPBR = 95; /* 250 kbps bit rate. */
  RSPI0.SPPCR.BYTE = 0; /* Normal mode. */
  RSPI0.SPDCR.BYTE = 0b100000; /* 1 Frame, Longword access. */
  RSPI0.SPCR2.BYTE = 0; /* Disable parity and interrupts. */
  RSPI0.SPSCR.BYTE = 0; /* Sequence length of one. */
  RSPI0.SPCMD0.WORD = 0x700; /* 8-bit data length, MSB first, SPI mode 0. */
  RSPI0.SPCR.BIT.MSTR = 1; /* SPI master mode. */
  RSPI0.SPCR.BIT.SPMS = 1; /* 3-wire mode. */

  RSPI0.SPCR.BIT.SPE = 1; /* Enable SPI. */
}

void sdcard_setup(void)
{
  char response;
  volatile int cycle;

  spi_setup();

  /* Send required dummy bits to inform SD card about clock. */
  PORTC.PODR.BIT.B0 = 1; /* Set CS. */
  spi_dummy_bytes(10); /* Need around 75 bits. */

  /* Send CMD0 to go to idle state. */
  cycle = 0;
  while (1) {
    response = spi_sdcard_command(0, 0);
    if (response == 0x01) { /* Card idle. */
      break;
    }
    cycle++;
    if (cycle > CMD0_FINISH_CYCLE_MAX) {
      goto sdcard_setup_end; /* Timeout! */
    }
  }

  /* Send CMD8 to check card type. */
  response = spi_sdcard_command(8, 0x1AA);
  spi_dummy_bytes(5); /* Flush rest of response. */
  if (response != 1) {
    /* Not SD version 2, older version 1 cards not supported! */
    goto sdcard_setup_end;
  }

  /* Send ACMD41 to initialize the card. */
  cycle = 0;
  while (1) {
    response = spi_sdcard_command(55, 0); /* CMD55 prefix for ACMDs. */
    spi_dummy_bytes(5); /* Flush rest of response. */
    if (response != 1) {
      goto sdcard_setup_end; /* Error, unexpected response. */
    }

    response = spi_sdcard_command(41, 0x40000000);
    spi_dummy_bytes(5); /* Flush rest of response. */
    if (response == 0) { /* Card ready. */
      break;
    }
    cycle++;
    if (cycle > ACMD41_FINISH_CYCLE_MAX) {
      goto sdcard_setup_end; /* Timeout! */
    }
  }

  sdcard_initialized = 1;
sdcard_setup_end:
  PORTC.PODR.BIT.B0 = 1; /* Set CS. */
}

int sdcard_sector_read(unsigned int no, unsigned char data[])
{
  char response;
  volatile int cycle;

  if (! sdcard_initialized) {
    return -1;
  }

  /* Send CMD16 to set block size. */
  response = spi_sdcard_command(16, 0x00000200); /* 512 bytes. */
  spi_dummy_bytes(5); /* Flush rest of response. */
  if (response != 0) {
    return -1;
  }

  /* Send CMD17 to read block 0. */
  response = spi_sdcard_command(17, no);
  if (response != 0) {
    return -1;
  }

  /* Wait for start of data code. */
  cycle = 0;
  while ((response = spi_transfer(0xFF)) != 0xFE) {
    cycle++;
    if (cycle > CMD17_FINISH_CYCLE_MAX) {
      return -1;
    }
  }

  /* Read the 512 bytes. */
  for (int i = 0; i < 512; i++) {
    response = spi_transfer(0xff);
    data[i] = response;
  }
  spi_dummy_bytes(2); /* Flush 16-bit CRC. */

  PORTC.PODR.BIT.B0 = 1; /* Set CS. */

  return 0;
}

