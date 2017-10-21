/*
 * Copyright (C) 2017  Francois Berder <fberder@outlook.fr>
 *
 * This file is part of fat16.
 *
 * fat16 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * fat16 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with fat16.  If not, see <http://www.gnu.org/licenses/>.
 */


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
