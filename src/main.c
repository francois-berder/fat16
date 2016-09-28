#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linux_hal.h"
#include "fat16.h"

#define DEFAULT_FS_PATH     "data/fs.img"


int main(int argc, char **argv)
{
    int ret;
    char fs_path[255];
    char buffer;

    if (argc < 2) {
        printf("Using default filesystem path: %s\n", DEFAULT_FS_PATH);
        strcpy(fs_path, DEFAULT_FS_PATH);
    } else {
        strcpy(fs_path, argv[1]);
    }

    // Load filesystem image
    if (linux_load_image(fs_path) < 0)
        return -1;
    printf("%s filesystem loaded.\n", fs_path);

    // FAT16 operations
    ret = fat16_init();
    printf("ret: %d\n", ret);

    ret = fat16_open("hello.txt", 'r');
    printf("ret: %d\n", ret);


    while (fat16_read(ret, &buffer, sizeof(buffer)) > 0)
        printf("%c", buffer);

    // Release filesystem image
    if (linux_release_image() < 0)
        return -1;

    printf("%s filesystem released.\n", fs_path);

    return 0;
}
