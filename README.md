# FAT16 driver

This aim of this project is to create a small program able to read FAT16 image on Linux. This project has also been ported to a PIC24, see [BoatController](https://github.com/francois-berder/BoatController).
The data folder contains images used to test the program.

#### Features

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
Concerning names, they must respect this format:
<1-8 chararcters>.<1-3 characters>

Example: DATA.TXT

File names are not case sensitive.

#### Build instructions

```sh
$ cmake .
$ make
```
This creates a shared library libfat16_driver.so, then the test suite must be run as root:

```sh
$ sudo ./run_test
```

