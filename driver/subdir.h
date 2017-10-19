#ifndef __FAT16_SUBDIR_H__
#define __FAT16_SUBDIR_H__

#include <stdbool.h>

int open_file_in_subdir(struct file_handle *handle, char *filename, bool read_mode);
int open_directory_in_subdir(struct file_handle *handle, char *dirname);

int create_file_in_subdir(struct file_handle *handle, char *filename);
int create_directory_in_subdir(struct file_handle *handle, char *dirname);

#endif
