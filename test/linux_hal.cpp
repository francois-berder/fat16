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


#include <stdio.h>
#include "linux_hal.h"

static FILE *image = NULL;

int linux_load_image(const char *path)
{
    if (path == NULL) {
        printf("linux_load_image: Cannot load image with null path\n");
        return -1;
    }

    if ((image = fopen(path, "r+")) == NULL) {
        printf("linux_load_image: Could not load file %s\n", path);
        return -1;
    }

    return 0;
}

int linux_release_image(void)
{
    if (image == NULL)
        return 0;

    if (fclose(image) == EOF)
        return -1;

    return 0;
}

int linux_read(void *buffer, uint32_t length)
{
    if (buffer == NULL) {
        printf("linux_read: Cannot read with null buffer\n");
        return -1;
    }

    if (length == 0)
        return 0;

    if (fread(buffer, sizeof(uint8_t), length, image) != length) {
        printf("linux_read: Error while reading %u bytes\n", length);
        return -1;
    }

    return 0;
}

int linux_read_byte(void *data)
{
    if (data == NULL) {
        printf("linux_read: Cannot read with null buffer\n");
        return -1;
    }

    if (fread(data, 1, 1, image) != 1) {
        printf("linux_read: Error while reading one byte\n");
        return -1;
    }

    return 0;
}

int linux_seek(uint32_t offset)
{
    return fseek(image, offset, SEEK_SET);
}

int linux_write(const void *buffer, uint32_t length)
{
    if (buffer == NULL) {
        printf("linux_write: Cannot write with null buffer\n");
        return -1;
    }

    if (length == 0)
        return 0;

    if (fwrite(buffer, sizeof(uint8_t), length, image) != length) {
        printf("linux_write: Error while writing %u bytes\n", length);
        return -1;
    }

    return fflush(image);
}

struct storage_dev_t linux_dev = {
    linux_read,
    linux_read_byte,
    linux_write,
    linux_seek
};
