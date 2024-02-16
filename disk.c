#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include "disk.h"
#include "mem.h"
#include "panic.h"

#define DISKS 4

typedef struct disk_s {
  char filename[PATH_MAX];
  uint8_t data[DISK_SIZE];
  bool write_changes;
} disk_t;

static disk_t disk[DISKS];



void disk_init(void)
{
  int i;
  /* Create what CP/M sees as empty disks. */
  for (i = 0; i < DISKS; i++) {
    memset(disk[i].data, 0xE5, DISK_SIZE);
  }
}



int disk_image_load(uint8_t disk_no, const char *filename, bool write_changes)
{
  FILE *fh;

  if (disk_no >= DISKS) {
    return -1;
  }

  fh = fopen(filename, "rb");
  if (fh == NULL) {
    return -1;
  }
  fread(disk[disk_no].data, sizeof(uint8_t), DISK_SIZE, fh);
  fclose(fh);

  disk[disk_no].write_changes = write_changes;
  strncpy(disk[disk_no].filename, filename, PATH_MAX);
  return 0;
}



void disk_sys_write(mem_t *mem, uint16_t address, uint16_t size)
{
  int i;
  for (i = 0; i < DISKS; i++) {
    /* Skip cold start loader in first sector. */
    mem_read_area(mem, address, &disk[i].data[DISK_SECTOR_SIZE], size);
  }
}



int disk_sector_read(uint8_t disk_no,
  uint8_t track_no, uint8_t sector_no, mem_t *mem, uint16_t address)
{
  if (disk_no >= DISKS) {
    return -1;
  }
  if (track_no >= DISK_TRACKS) {
    return -1;
  }
  if (sector_no == 0 || sector_no >= (DISK_SECTORS + 1)) {
    return -1;
  }

  mem_write_area(mem, address, &disk[disk_no].data
    [(track_no * DISK_SECTORS * DISK_SECTOR_SIZE) +
    ((sector_no - 1) * DISK_SECTOR_SIZE)], DISK_SECTOR_SIZE);

  return 0;
}



int disk_sector_write(uint8_t disk_no,
  uint8_t track_no, uint8_t sector_no, mem_t *mem, uint16_t address)
{
  FILE *fh;

  if (disk_no >= DISKS) {
    return -1;
  }
  if (track_no >= DISK_TRACKS) {
    return -1;
  }
  if (sector_no == 0 || sector_no >= (DISK_SECTORS + 1)) {
    return -1;
  }

  mem_read_area(mem, address, &disk[disk_no].data
    [(track_no * DISK_SECTORS * DISK_SECTOR_SIZE) +
    ((sector_no - 1) * DISK_SECTOR_SIZE)], DISK_SECTOR_SIZE);

  if (disk[disk_no].write_changes) {
    fh = fopen(disk[disk_no].filename, "wb");
    if (fh == NULL) {
      panic("fopen() failed for: %s\n", disk[disk_no].filename);
    }
    fwrite(disk[disk_no].data, sizeof(uint8_t), DISK_SIZE, fh);
    fclose(fh);
  }

  return 0;
}



