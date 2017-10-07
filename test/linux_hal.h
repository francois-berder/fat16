#ifndef LINUX_HAL_H
#define LINUX_HAL_H

#include <stdint.h>
#include "../driver/fat16.h"

#ifdef __cplusplus
extern "C" {
#endif

int linux_load_image(const char *path);
int linux_release_image(void);

extern struct storage_dev_t linux_dev;

#ifdef __cplusplus
}
#endif

#endif
