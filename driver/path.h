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


#ifndef __FAT16_PATH_H__
#define __FAT16_PATH_H__

#include <stdbool.h>

/**
 * @brief Convert filename to 8.3 FAT short name.
 *
 * Example "hello.txt" to "HELLO   TXT"
 *
 * @param[out] filename 11 long char array
 * @param[in] path arbitrary long string
 * @return 0 if successful, -1 otherwise
 */
int to_short_filename(char *short_filename, const char *long_filename);

/**
 * @brief Extract intermediate directories from path
 *
 * @param[out] subdir_name
 * @param[out] index
 * @param[in] path Must be a valid path
 * @retval 0 if successful
 * @retval -1 if an error occured
 * @retval -2 if no intermediate directory was found
 */
int get_subdir(char *subdir_name, uint16_t *index, const char *path);

bool is_in_root(const char *path);

#endif
