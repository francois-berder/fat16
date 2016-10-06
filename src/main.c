/* Tests for the FAT16 driver. */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linux_hal.h"
#include "fat16.h"

#define DEFAULT_FS_PATH     "data/fs.img"

typedef bool(*test_func)(void);

struct test_case {
    char *name;
    test_func f;
};

static bool test_init(void)
{
    return fat16_init() == 0;
}

static struct test_case tests[] = {
    { "init", test_init }
};

int main(int argc, char **argv)
{
    char fs_path[255];
    unsigned int i = 0;

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

    for (i = 0; i < sizeof(tests)/sizeof(struct test_case); ++i) {
        printf("===== test %u: %s =====\n\n", i+1, tests[i].name);
        if (tests[i].f())
            printf("PASS\n");
        else
            printf("FAIL\n");
    }

    // Release filesystem image
    if (linux_release_image() < 0)
        return -1;

    printf("%s filesystem released.\n", fs_path);

    return 0;
}
