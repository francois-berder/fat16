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

#endif
