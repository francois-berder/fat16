# FAT16 Reader

This aim of this project is to create a small program able to read FAT16 image on Linux which will be ported later on a PIC24.
The data folder contains images used to test the program.

#### Build instructions:

```sh
$ cmake .
$ make
```
This creates a shared library libfat16_driver.so, then the test suite must be run as root:

```sh
$ sudo ./run_test
```

