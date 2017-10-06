#ifndef __FAT16_PATH_H__
#define __FAT16_PATH_H__

/**
 * @brief Get filename as 8.3 short FAT name.
 *
 * Example "hello.txt" to "HELLO   TXT"
 *
 * @param[out] filename 11 long char array
 * @param[in] path arbitrary long string
 * @return 0 if successful, -1 otherwise
 */
int fat16_get_short_filename(char *filename, const char *path);

#endif
