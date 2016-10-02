#ifndef PIC24_HAL_H
#define PIC24_HAL_H

#include <stdint.h>

int pic24_read(uint8_t *buffer, uint32_t length);
int pic24_read_byte(uint8_t *data);
int pic24_seek(int offset);
int pic24_write(uint8_t *buffer, uint32_t length);

#endif
