#include <stdio.h>
#include <stdint.h>
#include "io.h"
#include "mem.h"
#include "disk.h"
#include "console.h"
#include "panic.h"



/* Input/Output ports as used in the CBIOS */
#define IO_PORT_VIRTUAL_CONSOLE_STATUS 0x00 /* Console status */
#define IO_PORT_VIRTUAL_CONSOLE_IO     0x01 /* Console input/output */
#define IO_PORT_VIRTUAL_DISK_SELECT    0x10 /* Disk   0 to 3 */
#define IO_PORT_VIRTUAL_DISK_TRACK     0x11 /* Track  0 to 76 */
#define IO_PORT_VIRTUAL_DISK_SECTOR    0x12 /* Sector 1 to 26 */
#define IO_PORT_VIRTUAL_DISK_DMA_L     0x13 /* DMA low address */
#define IO_PORT_VIRTUAL_DISK_DMA_H     0x14 /* DMA high address */
#define IO_PORT_VIRTUAL_DISK_IO        0x15 /* Perform disk I/O */



static uint8_t io_disk_select = 0;
static uint8_t io_disk_track  = 0;
static uint8_t io_disk_sector = 0;
static uint16_t io_disk_dma   = 0;
static uint8_t io_disk_status = 0;



uint8_t io_read(uint8_t port, uint8_t upper_address)
{
  switch (port) {
  case IO_PORT_VIRTUAL_CONSOLE_STATUS:
    return console_status();

  case IO_PORT_VIRTUAL_CONSOLE_IO:
    return console_read();

  case IO_PORT_VIRTUAL_DISK_IO:
    return io_disk_status;

  default:
    panic("Unknown IO port read: %02x (upper: %02x)\n", port, upper_address);
    break;
  }

  return 0;
}



void io_write(uint8_t port, uint8_t upper_address, uint8_t value, mem_t *mem)
{
  switch (port) {
  case IO_PORT_VIRTUAL_CONSOLE_IO:
    console_write(value);
    break;

  case IO_PORT_VIRTUAL_DISK_SELECT:
    io_disk_select = value;
    break;

  case IO_PORT_VIRTUAL_DISK_TRACK:
    io_disk_track = value;
    break;

  case IO_PORT_VIRTUAL_DISK_SECTOR:
    io_disk_sector = value;
    break;

  case IO_PORT_VIRTUAL_DISK_DMA_L:
    io_disk_dma = (io_disk_dma & 0xFF00) | value;
    break;

  case IO_PORT_VIRTUAL_DISK_DMA_H:
    io_disk_dma = (io_disk_dma & 0x00FF) | (value << 8);
    break;

  case IO_PORT_VIRTUAL_DISK_IO:
    if (value == 0x01) { /* Read */
      if (0 == disk_sector_read(io_disk_select, io_disk_track, io_disk_sector,
        mem, io_disk_dma)) {
        io_disk_status = 0;
      } else {
        io_disk_status = 1;
      }

    } else if (value == 0x02) { /* Write */
      if (0 == disk_sector_write(io_disk_select, io_disk_track, io_disk_sector,
        mem, io_disk_dma)) {
        io_disk_status = 0;
      } else {
        io_disk_status = 1;
      }

    } else {
      panic("Unhandled virtual disk IO: %02x\n", value);
    }
    break;

  default:
    panic("Unknown IO port write: %02x (upper: %02x) (value: %02x)\n",
      port, upper_address, value);
    break;
  }
}



