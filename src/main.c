#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_FS_PATH     "data/fs.img"


int main(int argc, char **argv)
{
    char fs_path[255];

    if (argc < 2) {
        printf("Using default filesystem path: %s\n", DEFAULT_FS_PATH);
        strcpy(fs_path, DEFAULT_FS_PATH);
    } else {
        strcpy(fs_path, argv[1]);
    }


    return 0;
}
