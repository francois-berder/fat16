# FAT16 driver [![Build Status](https://travis-ci.org/francois-berder/fs.svg?branch=master)](https://travis-ci.org/francois-berder/fs)

This aim of this project is to create a small program able to read FAT16 image on Linux. This project has also been ported to a PIC24, see [BoatController](https://github.com/francois-berder/BoatController).
The data folder contains images used to test the program.

## Features

The driver can:
   - create/delete file located in the root directory
   - list files in the root directory
   - read to a file (file can be opened several times in reading mode)
   - write to a file, its contents are erased. A file can be opened

This driver cannot:
   - create/delete directories
   - read/write/create/delete a file which is not in the root directory
   - handle long names

Hence, a maximum of 512 files can be handled by this driver on a FAT16 partition.
Concerning names, they must respect the format described here: https://en.wikipedia.org/wiki/8.3_filename. Hence, all valid filenames can be stored as an array of char of length  13 characters (12 bytes for the filename and one byte for the null character).

## Build instructions

You need a relatively recent versions of gcc as tests are written using C++11 features. It is known not to compile on Ubuntu 14.04 LTS. GCC 5+ work fine.

```sh
$ cmake .
$ make
```
This creates a shared library libfat16_driver.so, then the test suite must be run as root:

```sh
$ sudo ./run_test
```

## Examples

Reading a file:
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
    uint16_t i = 0;
    while(i >= 0) {
        i = fat16_ls(i, filename);
        if (i >= 0)
            printf("%s\n", filename);
    }
}
```
