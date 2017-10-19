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
