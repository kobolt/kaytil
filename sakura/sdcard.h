#ifndef _SDCARD_H
#define _SDCARD_H

void sdcard_setup(void);
int sdcard_sector_read(unsigned int no, unsigned char data[]);

#endif /* _SDCARD_H */
