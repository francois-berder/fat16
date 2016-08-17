#include <stdio.h>
#include "linux_hal.h"


int linux_load_image(char *path)
{
    printf("linux_load_image: not implemented\n");
    return -1;
}

int linux_release_image(void)
{
    printf("linux_release_image: not implemented\n");
    return -1;
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
