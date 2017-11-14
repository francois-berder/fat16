/*
 * Copyright (C) 2017  Francois Berder <fberder@outlook.fr>
 *
 * This file is part of fat16.
 *
 * fat16 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * fat16 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with fat16.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef FAT16_H
#define FAT16_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum FAT_ERROR {
    INVALID_JUMP_INSTRUCTION,
    INVALID_BYTES_PER_SECTOR,
    INVALID_SECTOR_PER_CLUSTER,
    INVALID_BYTES_PER_CLUSTER,
    INVALID_RESERVED_SECTOR_COUNT,
    INVALID_ROOT_ENTRY_COUNT,
    INVALID_SECTOR_COUNT,
    INVALID_FAT_TYPE
};

struct storage_dev_t {
    int (*read)(void *buffer, uint32_t length);
    int (*read_byte)(void *data);
    int (*write)(const void *buffer, uint32_t length);
    int (*seek)(uint32_t offset);
};

/**
 * @brief Initialise the FAT16 driver.
 *
 * It reads the BPB, initialises internal variables.
 * It must be called before doing any other operations.
 *
 * @param[in] dev
 * @param[in] offset Absolute position of the first byte which belongs to a FAT16 partition
 *                   (obtained by reading the MBR, can be 0 if reading from a FAT16 image).
 * @return 0 if successful, -1 otherwise
 */
int __attribute__((visibility("default"))) fat16_init(struct storage_dev_t dev, uint32_t offset);

/**
 * @brief Open a file.
 *
 * A file can be opened multiple times for reading. But, it can be opened only
 * once at a time for writing.
 *
 * If a file opened in write mode does not exist, it is created. On the other
 * hand, if it exists, the content will be deleted first.
 *
 * @param[in] filepath
 * @param[in] mode Can be 'r' (read only), 'w' (write only), 'a' (append, write only)
 * @return A handle of the file (positive integer) if it could open it.
 * Otherwise, a negative value is returned.
 */
int __attribute__((visibility("default"))) fat16_open(const char *filepath, char mode);

/**
 * @brief Read data from file.
 *
 * If the end of file is reached and there are no more bytes to be read, this
 * function returns 0.
 *
 * @param[in] handle Positive number returned by fat16_open.
 * @param[out] buffer Pointer to a buffer.
 * @param[in] Number of bytes to read.
 * @return Number of bytes read from file. The return value might be less than
 * count because the end of the file is reached.
 */
int __attribute__((visibility("default"))) fat16_read(uint8_t handle, void *buffer, uint32_t count);

/**
 * @brief Write data to file.
 *
 * @param[in] handle Positive number returned by fat16_open.
 * @param[in] buffer Pointer to a buffer.
 * @param[in] Number of bytes to read.
 * @return Number of bytes written to file. The return value might be less than
 * count if there is no space left.
 */
int __attribute__((visibility("default"))) fat16_write(uint8_t handle, const void *buffer, uint32_t count);

/**
 * @brief Release the handle.
 *
 * @param[in] handle Positive number returned by fat16_open
 * @return 0 if successful, -1 otherwise
 */
int __attribute__((visibility("default"))) fat16_close(uint8_t handle);

/**
 * @brief Delete a file.
 *
 * The file must not be opened for reading or writing.
 *
 * @param[in] filepath
 * @return 0 if successful, -1 otherwise
 */
int __attribute__((visibility("default"))) fat16_rm(const char *filepath);

/**
 * @brief Gives the name of a file in a directory.
 *
 * By repeatedly calling this function in this manner:
 *
 * @code{.c}
 * char filename[13];
 * uint32_t i = 0;      // Must be initialised to 0
 * while(fat16_ls(&i, filename, "/") == 1) {
 *      printf("%s\n", filename);
 * }
 * @endcode
 *
 * Do not modify the directory (creating/deleting files) while your are
 * iterating through the list of files in this directory.
 *
 * @param[in/out] index Do not change the value of this variable between calls to fat16_ls
 * @param[out] filename Name An array of 13 characters is sufficient
 * @param[in] dirpath
 * @retval 1 if a filename was retrieved with success
 * @retval 0 if the end of the file list was reached
 * @retval -1 if an error occurs
 */
int __attribute__((visibility("default"))) fat16_ls(uint32_t *index, char *filename, const char *dirpath);

/**
 * @brief Create a directory
 *
 * @param[in] dirpath
 * @return 0 if successful, -1 otherwise
 */
int __attribute__((visibility("default"))) fat16_mkdir(const char *dirpath);

/**
 * @brief Delete a directory
 *
 * The directory must be empty.
 *
 * @param[in] dirpath
 * @return 0 if successful, -1 otherwise
 */
int __attribute__((visibility("default"))) fat16_rmdir(const char *dirpath);

#ifdef __cplusplus
}
#endif

#endif
