# FAT16 Reader

This aim of this project is to create a small program able to read FAT16 image on Linux which will be ported later on a PIC24.

The data folder contains images used to test the program.

#### Build instructions:

```sh
$ cmake .
$ make
$ ./fs
```

The path to a specific fat16 image can be specified, otherwise, it will read the image at data/fs.img.
