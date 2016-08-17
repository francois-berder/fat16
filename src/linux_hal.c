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
    printf("linux_read: not implemented\n");
    return -1;
}

int linux_read_byte(uint8_t *data)
{
    printf("linux_read_byte: not implemented\n");
    return -1;
}
