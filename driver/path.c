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

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "path.h"

static bool is_character_valid(char c)
{
    return ('A' <= c && c <= 'Z')
           || ('0' <= c && c <= '9')
           || c == '!'
           || c == '#'
           || c == '$'
           || c == '%'
           || c == '&'
           || c == '\''
           || c == '('
           || c == ')'
           || c == '-'
           || c == '@'
           || c == '^'
           || c == '_'
           || c == '`'
           || c == '{'
           || c == '}'
           || c == '~';
}

int to_short_filename(char *short_filename, const char *long_filename)
{
    uint8_t i = 0;
    uint8_t sep = 0;

    if (long_filename[0] == '/')
        ++long_filename;

    if (long_filename[0] == '\0')
        return -1;

    /* Find position of . (marker between name and extension) */
    for (i = 0; i < 9; ++i) {
        /* If that is already the end of file, fill with spaces and exit */
        if (long_filename[i] == '\0') {
            memset(&short_filename[i], ' ', 11 - i);
            return 0;
        }

        if (long_filename[i] == '.') {
            sep = i;
            break;
        }

        if (!is_character_valid(long_filename[i])) {
            return -1;
        }

        short_filename[i] = long_filename[i];
    }

    /*
     * If it cannot find . in the first 9 characters then the name is more
     * than 8 characters long which is forbidden.
     */
    if (i == 9)
        return -1;

    memset(&short_filename[sep], ' ', 8 - sep);

    /* Copy extension */
    for (i = 0; i < 3; ++i) {
        if (long_filename[sep + 1 + i] == '\0') {
            break;
        }

        if (!is_character_valid(long_filename[sep + 1 + i])) {
            return -1;
        }

        short_filename[8 + i] = long_filename[sep + 1 + i];
    }
    /* Check that extension consists of no more than 3 characters */
    if (i == 3 && long_filename[sep + 4] != '\0')
        return -1;

    memset(&short_filename[8+i], ' ', 3-i);

    return 0;
}

int get_subdir(char *subdir_name, uint16_t *index, const char *path)
{
    const uint16_t beg = *index;
    unsigned int len = 0;

    if (path[beg] != '/')
        return -1;

    len++; /* Skip first slash */
    while (path[beg + len] != '\0') {
        if (path[beg + len] == '/')
            break;

        ++len;
    }
    /* Check if path is an intermediate directory */
    if (path[beg + len] != '/')
        return -2;

    if (len > 12)
        return -1;

    memcpy(subdir_name, &path[beg], len);
    subdir_name[len] = '\0';

    *index = beg + len;
    return 0;
}

bool is_in_root(const char *path)
{
    char subdir_name[13];
    uint16_t index = 0;

    return get_subdir(subdir_name, &index, path) < 0;
}
