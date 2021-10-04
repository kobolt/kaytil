#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "sdcard.h"

/* A simple cache that keeps the lowest sectors cached,
   which should hopefully include the frequently accessed FAT tables. */
#define FAT_CACHE_SIZE 32
#define FAT_SECTOR_SIZE 512

typedef struct cache_s {
  bool filled;
  uint8_t data[FAT_SECTOR_SIZE];
} cache_t;

static cache_t cache[FAT_CACHE_SIZE];
static uint16_t cache_cluster;
static uint8_t temp[FAT_SECTOR_SIZE];
static uint8_t *sector;



static int sector_read(unsigned int no)
{
  if (no >= FAT_CACHE_SIZE) {
    /* Direct read. */
    sector = &temp[0];
    return sdcard_sector_read(no, temp);

  } else {
    /* Cached read. */
    sector = &cache[no].data[0];
    if (cache[no].filled) {
      return 0;
    } else {
      int result = sdcard_sector_read(no, cache[no].data);
      if (result == 0) {
        cache[no].filled = true;
      }
      return result;
    }
  }
}



void fat16_cache_clear(void)
{
  cache_cluster = 0;
  for (int i = 0; i < FAT_CACHE_SIZE; i++) {
    cache[i].filled = false;
    memset(cache[i].data, 0xE5, FAT_SECTOR_SIZE);
  }
}



int fat16_read(char *in_file, unsigned int in_offset,
  uint8_t out_data[], unsigned int out_size)
{
  volatile uint32_t vbr_offset;
  volatile uint32_t fat_offset;
  volatile uint32_t root_dir_offset;
  volatile uint32_t data_offset;

  volatile uint8_t partition_type;
  volatile uint8_t sectors_per_cluster;
  volatile uint16_t reserved_sectors;
  volatile uint8_t no_of_fats;
  volatile uint16_t root_entries;
  volatile uint16_t sectors_per_fat;

  char file_name[13]; /* 8.3 format at NULL byte. */
  uint16_t file_cluster;

  uint16_t cluster = 0; /* Target cluster for read. */
  uint16_t cluster_end_indicator;
  uint16_t cluster_fat;
  uint16_t cluster_offset;

  if (out_size > FAT_SECTOR_SIZE) {
    return -1; /* Reads larger than sector size is not supported. */
  }

  if (cache_cluster == 0) {
    /* No target cluster cached, read again .*/

    if (sector_read(0) != 0) {
      return -1;
    }

    partition_type = *(sector+450);
    vbr_offset = *(sector+454);

    if (partition_type != 0x04 && partition_type != 0x06) {
      return -1;
    }

    if (sector_read(vbr_offset) != 0) {
      return -1;
    }

    sectors_per_cluster = *(sector+13);
    reserved_sectors    = *(sector+14) + (*(sector+15) << 8);
    no_of_fats          = *(sector+16);
    root_entries        = *(sector+17) + (*(sector+18) << 8);
    sectors_per_fat     = *(sector+22) + (*(sector+23) << 8);

    fat_offset = vbr_offset + reserved_sectors;
    root_dir_offset = fat_offset + (sectors_per_fat * no_of_fats);
    data_offset = root_dir_offset + ((root_entries * 32) / FAT_SECTOR_SIZE);

    for (uint32_t offset = root_dir_offset; offset < data_offset; offset++) {
      if (sector_read(offset) != 0) {
        return -1;
      }

      for (int i = 0; i < FAT_SECTOR_SIZE; i += 32) {
        if (*(sector+i) == '\0') {
          break; /* End of files. */
        }

        if (*(sector+i+11) & 0x10 || *(sector+i+11) & 0x08) {
          continue; /* Subdirectory or volume label. */
        }

        int n = 0;
        for (int j = 0; j < 8; j++) {
          if (*(sector+i+j) == ' ') {
            break;
          }
          file_name[n] = *(sector+i+j);
          n++;
        }

        file_name[n] = '.';
        n++;

        for (int j = 0; j < 3; j++) {
          if (*(sector+i+8+j) == ' ') {
            break;
          }
          file_name[n] = *(sector+i+8+j);
          n++;
        }

        file_name[n] = '\0';

        file_cluster = *(sector+i+26) + (*(sector+i+27) << 8);
        /* File size is ignored since sectors should be aligned anyway. */

        if (strcmp(in_file, file_name) == 0) {
          cluster = file_cluster;
          cache_cluster = cluster;
        }
      }
    }
  } else {
    /* Re-use previous target cluster. */
    cluster = cache_cluster;
  }

  if (cluster > 0) {
    if (sector_read(fat_offset) != 0) {
      return -1;
    }

    cluster_end_indicator = *(sector+2) + (*(sector+3) << 8);

    unsigned int bytes_read = 0;
    while ((cluster >> 3) != (cluster_end_indicator >> 3)) {

      for (int s = 0; s < sectors_per_cluster; s++) {
        if (in_offset < (bytes_read + FAT_SECTOR_SIZE)) {
          if (sector_read(data_offset + 
            ((cluster - 2) * sectors_per_cluster) + s) != 0) {
            return -1;
          }

          memcpy(out_data, &*(sector+in_offset % FAT_SECTOR_SIZE), out_size);
          return 0;
        }
        bytes_read += FAT_SECTOR_SIZE;
      }

      /* Calculate next cluster. */
      cluster_fat = cluster / 256;
      cluster_offset = (cluster % 256) * 2;

      if (sector_read(fat_offset + cluster_fat) != 0) {
        return -1;
      }
      cluster = *(sector+cluster_offset) + (*(sector+cluster_offset+1) << 8);
    }
  }

  return -1; /* File not found! */
}

