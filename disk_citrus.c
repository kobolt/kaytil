#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "led.h"
#include "disk.h"
#include "mem.h"

extern uint8_t binary_cpm22_bin_start[];

extern uint8_t binary_citrus_disk_a_img_start[];
extern uint8_t binary_citrus_disk_b_img_start[];
extern uint8_t binary_citrus_disk_c_img_start[];
extern uint8_t binary_citrus_disk_d_img_start[];
extern uint8_t binary_citrus_disk_a_img_end[];
extern uint8_t binary_citrus_disk_b_img_end[];
extern uint8_t binary_citrus_disk_c_img_end[];
extern uint8_t binary_citrus_disk_d_img_end[];



int disk_sector_read(uint8_t disk_no,
  uint8_t track_no, uint8_t sector_no, mem_t *mem, uint16_t address)
{
  uint32_t index;
  uint32_t disk_size;

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

  led_command(LED_ON);

  index = (track_no * DISK_SECTORS * DISK_SECTOR_SIZE) +
          ((sector_no - 1) * DISK_SECTOR_SIZE);

  switch (disk_no) {
  case 0:
    disk_size = binary_citrus_disk_a_img_end - binary_citrus_disk_a_img_start;
    if (index + DISK_SECTOR_SIZE > disk_size) {
      /* Return uninitialized bytes on overflow. */
      for (int i = 0; i < DISK_SECTOR_SIZE; i++) {
        mem_write(mem, address + i, 0xE5);
      }
    } else {
      mem_write_area(mem, address,
        &binary_citrus_disk_a_img_start[index], DISK_SECTOR_SIZE);
    }
    break;

  case 1:
    disk_size = binary_citrus_disk_b_img_end - binary_citrus_disk_b_img_start;
    if (index + DISK_SECTOR_SIZE > disk_size) {
      /* Return uninitialized bytes on overflow. */
      for (int i = 0; i < DISK_SECTOR_SIZE; i++) {
        mem_write(mem, address + i, 0xE5);
      }
    } else {
      mem_write_area(mem, address,
        &binary_citrus_disk_b_img_start[index], DISK_SECTOR_SIZE);
    }
    break;

  case 2:
    disk_size = binary_citrus_disk_c_img_end - binary_citrus_disk_c_img_start;
    if (index + DISK_SECTOR_SIZE > disk_size) {
      /* Return uninitialized bytes on overflow. */
      for (int i = 0; i < DISK_SECTOR_SIZE; i++) {
        mem_write(mem, address + i, 0xE5);
      }
    } else {
      mem_write_area(mem, address,
        &binary_citrus_disk_c_img_start[index], DISK_SECTOR_SIZE);
    }
    break;

  case 3:
    disk_size = binary_citrus_disk_d_img_end - binary_citrus_disk_d_img_start;
    if (index + DISK_SECTOR_SIZE > disk_size) {
      /* Return uninitialized bytes on overflow. */
      for (int i = 0; i < DISK_SECTOR_SIZE; i++) {
        mem_write(mem, address + i, 0xE5);
      }
    } else {
      mem_write_area(mem, address,
        &binary_citrus_disk_d_img_start[index], DISK_SECTOR_SIZE);
    }
    break;

  default:
    return -1;
  }

  /* Only turned off if OK! */
  led_command(LED_OFF);

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



