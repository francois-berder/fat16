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


#ifndef __FAT16_SUBDIR_H__
#define __FAT16_SUBDIR_H__

#include <stdbool.h>

/**
 * @brief Create a file in a subdirectory
 *
 * No other entries with this name must exist.
 *
 * @param[in|out] handle
 * @param[in] filename 8.3 short name
 * @return 0 if successful, -1 otherwise
 */
int create_file_in_subdir(struct entry_handle *handle, char *filename);

/**
 * @brief Create a directory in a subdirectory
 *
 * @param[in|out] handle
 * @param[in] dirname 8.3 short name
 * @return 0 if successful, -1 otherwise
 */
int create_directory_in_subdir(struct entry_handle *handle, char *dirname);

/**
 * @brief Open a file in a subdirectory
 *
 * No file will be created by this function. It must already exist in the
 * directory
 *
 * @param[in|out] handle
 * @param[in] filename 8.3 short name
 * @param[in] mode
 * @return 0 if successful, -1 otherwise
 */
int open_file_in_subdir(struct entry_handle *handle, char *filename, char mode);

/**
 * @brief Open a directory in a subdirectory
 *
 * No directory will be created by this function. It must already
 * exist in the directory
 *
 * @param[in|out] handle
 * @param[in] dirname 8.3 short name
 * @return 0 if successful, -1 otherwise
 */
int open_directory_in_subdir(struct entry_handle *handle, char *dirname);

/**
 * @brief Delete a file in a subdirectory
 *
 * @param[in] handle
 * @param[in] filename 8.3 short name
 * @return 0 if successful, -1 otherwise
 */
int delete_file_in_subdir(struct entry_handle *handle, char *filename);

/**
 * @brief Delete a directory in a subdirectory
 *
 * @param[in] handle
 * @param[in] dirname 8.3 short name
 * @return 0 if successful, -1 otherwise
 */
int delete_directory_in_subdir(struct entry_handle *handle, char *dirname);

/**
 * @brief Check a directory is empty
 *
 * @param[in] handle
 * @return True if empty, False otherwise
 */
bool is_subdir_empty(struct entry_handle *handle);

/**
 * @brief
 *
 *
 * @param[in|out] index
 * @param[out] name
 * @param[in] dirpath
 * @retval 1
 * @retval 0
 * @retval -1
 */
int ls_in_subdir(uint32_t *index, char *name, struct entry_handle *handle);

#endif
