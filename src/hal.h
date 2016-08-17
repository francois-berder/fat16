#ifndef HAL_H
#define HAL_H

#include <stdint.h>

#define PLATFORM_LINUX
//#define PLATFORM_PIC24

int hal_read(uint8_t *buffer, uint32_t length);
int hal_read_byte(uint8_t *data);

#endif
