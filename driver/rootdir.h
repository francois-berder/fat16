#ifndef __FAT16_ROOTDIR_H__
#define __FAT16_ROOTDIR_H__

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Create a file in the root directory
 *
 * @param[in] name 8.3 short filename
 * @retval -1 if there is no available entry in the root directory,
 * @reval 0 if successful
 */
int create_file_in_root(char *filename);

/**
 * @brief Create a file in the root directory
 *
 * @param[in] name 8.3 short filename
 * @retval -1 if there is no available entry in the root directory,
 * @reval 0 if successful
 */
int create_directory_in_root(char *dirname);

/**
 * @brief Open a file located in the root directory
 *
 * @param[out] handle
 * @param[in] name 8.3 short filename
 * @param[in] read_mode
 * @return 0 if successful, -1 otherwise
 */
int open_file_in_root(struct file_handle *handle, char *filename, bool read_mode);

/**
 * @brief Delete a file.
 *
 * Remove the entry from the and mark all clusters used by this file as
 * available. It does not clear the data region.
 *
 * @param[in] fat_filename Name of the filename in 8.3 format
 * @return 0 if successful, -1 otherwise
 */
int delete_file_in_root(char *filename);

#endif
