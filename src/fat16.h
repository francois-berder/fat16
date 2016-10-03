#ifndef FAT16_H
#define FAT16_H

#include <stdint.h>

enum FAT_ERROR
{
    NO_ERROR,
    INVALID_JUMP_INSTRUCTION,
    INVALID_BYTES_PER_SECTOR,
    INVALID_SECTOR_PER_CLUSTER,
    INVALID_BYTES_PER_CLUSTER,
    INVALID_RESERVED_SECTOR_COUNT,
    INVALID_ROOT_ENTRY_COUNT,
    INVALID_SECTOR_COUNT,
    INVALID_FAT_TYPE
};


int fat16_init(void);

/**
 * @brief Open a file.
 *
 * No sub directories are supported, so the path is a slash followed by the
 * name of a file.
 * A file can be opened multiple times for reading. But, it can be opened only
 * once at a time for writing.
 *
 * @param[in] path Path to a file
 * @param[in] mode Can be 'r' or 'w'
 * @return A handle of the file (positive integer) if it could open it.
 * Otherwise, a negative value is returned.
 */
int fat16_open(char *filename, char mode);

int fat16_read(uint8_t handle, char *buffer, uint32_t count);

int fat16_delete(char *filename);

#endif
