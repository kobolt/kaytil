#ifndef _FAT16_H
#define _FAT16_H

#include <stdint.h>

void fat16_cache_clear(void);
int fat16_read(char *in_file, unsigned int in_offset,
  uint8_t out_data[], unsigned int out_size);

#endif /* _FAT16_H */
