#ifndef __FAT16_SUBDIR_H__
#define __FAT16_SUBDIR_H__

int open_directory_in_subdir(struct file_handle *handle, char *dirname);

int create_directory_in_subdir(struct file_handle *handle, char *dirname);

#endif
