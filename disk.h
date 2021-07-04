#ifndef _DISK_H
#define _DISK_H

#include <stdint.h>
#include <stdbool.h>
#include "mem.h"

/* IBM 3740 8-inch floppy emulation */
#define DISK_TRACKS 77
#define DISK_SECTORS 26
#define DISK_SECTOR_SIZE 128
#define DISK_SIZE (DISK_TRACKS * DISK_SECTORS * DISK_SECTOR_SIZE) /* 256256 */

void disk_init(void);
int disk_image_load(uint8_t disk_no, const char *filename, bool write_changes);
void disk_sys_write(mem_t *mem, uint16_t address, uint16_t size);
int disk_sector_read(uint8_t disk_no,
  uint8_t track_no, uint8_t sector_no, mem_t *mem, uint16_t address);
int disk_sector_write(uint8_t disk_no,
  uint8_t track_no, uint8_t sector_no, mem_t *mem, uint16_t address);

#endif /* _DISK_H */
