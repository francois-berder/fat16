#ifndef HAL_H
#define HAL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int hal_read(uint8_t *buffer, uint32_t length);
int hal_read_byte(uint8_t *data);
int hal_seek(int offset);
int hal_write(const uint8_t *buffer, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif
