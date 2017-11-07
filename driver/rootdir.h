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


#ifndef __FAT16_ROOTDIR_H__
#define __FAT16_ROOTDIR_H__

/**
 * @brief Create a file in the root directory
 *
 * @param[in] filename 8.3 short name
 * @retval -1 if there is no available entry in the root directory,
 * @reval 0 if successful
 */
int create_file_in_root(char *filename);

/**
 * @brief Create a directory in the root directory
 *
 * @param[in] dirname 8.3 short name
 * @retval -1 if there is no available entry in the root directory,
 * @reval 0 if successful
 */
int create_directory_in_root(char *dirname);

/**
 * @brief Open a file located in the root directory
 *
 * @param[out] handle
 * @param[in] filename 8.3 short name
 * @param[in] mode
 * @return 0 if successful, -1 otherwise
 */
int open_file_in_root(struct entry_handle *handle, char *filename, char mode);

/**
 * @brief Open a directory located in the root directory
 *
 * @param[out] handle
 * @param[in] dirname 8.3 short name
 * @return 0 if successful, -1 otherwise
 */
int open_directory_in_root(struct entry_handle *handle, char *dirname);

/**
 * @brief Delete a file.
 *
 * Remove the entry from the and mark all clusters used by this file as
 * available. It does not clear the data region.
 *
 * @param[in] filename 8.3 short name
 * @return 0 if successful, -1 otherwise
 */
int delete_file_in_root(char *filename);

/**
 * @brief Delete a directory
 *
 * @param dirname 8.3 short name
 * @return 0 if successful, -1 otherwise
 */
int delete_directory_in_root(char *dirname);

int ls_in_root(uint32_t *index, char *filename);

#endif
