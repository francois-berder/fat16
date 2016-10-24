#ifndef LINUX_HAL_H
#define LINUX_HAL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int linux_load_image(const char *path);
int linux_release_image(void);

int linux_read(uint8_t *buffer, uint32_t length);
int linux_read_byte(uint8_t *data);
int linux_seek(int offset);
int linux_write(uint8_t *buffer, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif
