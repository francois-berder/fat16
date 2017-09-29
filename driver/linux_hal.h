#ifndef LINUX_HAL_H
#define LINUX_HAL_H

#include <stdint.h>
#include "hal.h"

#ifdef __cplusplus
extern "C" {
#endif

int linux_load_image(const char *path);
int linux_release_image(void);

#ifdef __cplusplus
}
#endif

#endif
