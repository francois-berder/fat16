#include <stdio.h>
#include "linux_hal.h"

static FILE *image = NULL;

int linux_load_image(char *path)
{
    if (path == NULL) {
        printf("linux_load_image: Cannot load image with null path\n");
        return -1;
    }

    if ((image = fopen(path, "rb")) == NULL) {
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

int linux_read(uint8_t *buffer, uint32_t length)
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

int linux_read_byte(uint8_t *data)
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
