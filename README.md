# FAT16 driver

This aim of this project is to create a small program able to read FAT16 image on Linux. This project has also been ported to a PIC24, see [BoatController](https://github.com/francois-berder/BoatController).
The data folder contains images used to test the program.

## Features

The driver can:

- list files in a directory
- read to a file (a file can be opened several times in reading mode)
- write to a file: any previous contents are erased. A file cannot be read while it is opened in write mode.
- append to a file: similar to write mode but any previous content is preserved and writing happen at the end.
- create/delete directories

This driver cannot handle long names.

Names of files and directories must respect the format described here: [https://en.wikipedia.org/wiki/8.3_filename](https://en.wikipedia.org/wiki/8.3_filename). Hence, all valid names can be stored as an array of char of length 13 characters (12 bytes for the filename and one byte for the null character).

## Build instructions

```sh
make
```

This creates a shared library ```libfat16_driver.so``` in the ```lib``` folder.
The test suite must be run as root:

```sh
sudo ./bin/run_test
```

## Integration in an application

You will need to implement the following functions to construct a ```struct storage_dev_t```:

```c
    int (*read)(void *buffer, uint32_t length);
    int (*read_byte)(void *data);
    int (*write)(const void *buffer, uint32_t length);
    int (*seek)(uint32_t offset);
```

You will also need to find out where the fat16 partition starts. If it is a FAT16 image, the address is most likely 0. Otherwise, read the MBR to get the first sector of a FAT16 partition, multiplied by 512 (bytes per sector) and pass it to ```fat16_init```.

On some compilers such as Microchip XC16, some features from C99 such as printing ```uint32_t``` are not supported
so you may have to change the format in debug print.

## Examples

Printing content of a file:

```c
void print_file_content(void)
{
    int fd = fat16_open("DATA.TXT", 'r');
    if (fd < 0) {
        fprintf(stderr, "Failed to read DATA.TXT\n");
        return;
    }

    while (1) {
        char buffer[256];
        int i, n;
        n = fat16_read(fd, buffer, sizeof(buffer));
        if (n == 0) {   /* End of file reached */
            break;
        } else if (n < 0) {
            fprintf(stderr, "Error %d while reading\n", -n);
            break;
        } else {
            for (i = 0; i < n; ++i)
                printf("%c", buffer[i]);
        }
    }

    fat16_close(fd);
}
```

Listing files in root directory:

```c
void list_files(void)
{
    char filename[13];
    uint32_t i = 0;
    while(fat16_ls(&i, filename, "/") == 1) {
        printf("%s\n", filename);
    }
}
```
