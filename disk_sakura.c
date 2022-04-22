#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include "disk.h"
#include "mem.h"
#include "panic.h"
#include "fat16.h"
#include "led.h"

extern unsigned char binary_cpm22_bin_start[];

#define DISKS 4

static uint8_t disk_data[DISK_SECTOR_SIZE];
static int current_disk = -1;



int disk_sector_read(uint8_t disk_no,
  uint8_t track_no, uint8_t sector_no, mem_t *mem, uint16_t address)
{
  char *file = NULL;

  if (track_no >= DISK_TRACKS) {
    return -1;
  }
  if (sector_no == 0 || sector_no >= (DISK_SECTORS + 1)) {
    return -1;
  }

  /* Return CP/M copy when reading the disk system sectors. */
  if (track_no == 0 && sector_no >= 2) {
    mem_write_area(mem, address,
      &binary_cpm22_bin_start[(sector_no - 2) * DISK_SECTOR_SIZE],
        DISK_SECTOR_SIZE);
    return 0;
  } else if (track_no == 1) {
    mem_write_area(mem, address,
      &binary_cpm22_bin_start[((DISK_SECTORS - 1) * DISK_SECTOR_SIZE) +
        (sector_no - 1) * DISK_SECTOR_SIZE], DISK_SECTOR_SIZE);
    return 0;
  }

  /* Since a new image file will be used, clear the cache on disk change. */
  if (disk_no != current_disk) {
    current_disk = disk_no;
    fat16_cache_clear();
  }

  switch (disk_no) {
  case 0:
    led_d1_command(LED_ON);
    file = "A.IMG";
    break;
  case 1:
    led_d2_command(LED_ON);
    file = "B.IMG";
    break;
  case 2:
    led_d3_command(LED_ON);
    file = "C.IMG";
    break;
  case 3:
    led_d4_command(LED_ON);
    file = "D.IMG";
    break;
  default:
    return -1;
  }

  if (fat16_read(file,
    (track_no * DISK_SECTORS * DISK_SECTOR_SIZE) +
    ((sector_no - 1) * DISK_SECTOR_SIZE),
    disk_data, DISK_SECTOR_SIZE) != 0) {

    /* In case of any error, return uninitialized bytes. */
    for (int i = 0; i < DISK_SECTOR_SIZE; i++) {
      disk_data[i] = 0xE5;
    }
  }

  mem_write_area(mem, address, disk_data, DISK_SECTOR_SIZE);

  switch (disk_no) {
  case 0:
    led_d1_command(LED_OFF);
    break;
  case 1:
    led_d2_command(LED_OFF);
    break;
  case 2:
    led_d3_command(LED_OFF);
    break;
  case 3:
    led_d4_command(LED_OFF);
    break;
  default:
    return -1;
  }

  return 0;
}



int disk_sector_write(uint8_t disk_no,
  uint8_t track_no, uint8_t sector_no, mem_t *mem, uint16_t address)
{
  /* Not supported. */
  (void)disk_no;
  (void)track_no;
  (void)sector_no;
  (void)mem;
  (void)address;
  return 0;
}



