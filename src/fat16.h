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


/**
 * @brief Initialise the FAT16 driver.
 *
 * It reads the BPB, initialise internal variables.
 * It must be called before doing any other operations.
 *
 * @return 0 if successful, -1 otherwise
 */
int fat16_init(void);

/**
 * @brief Open a file.
 *
 * No sub directories are supported, so the path is the name of the file.
 *
 * A file can be opened multiple times for reading. But, it can be opened only
 * once at a time for writing.
 *
 * If a file opened in write mode does not exist, it is created. On the other
 * hand, if it exists, the content will be deleted first.
 *
 * @param[in] filename
 * @param[in] mode Can be 'r' or 'w'
 * @return A handle of the file (positive integer) if it could open it.
 * Otherwise, a negative value is returned.
 */
int fat16_open(char *filename, char mode);

/**
 * @brief Read data from file.
 *
 * @param[in] handle Positive number returned by fat16_init.
 * @param[out] buffer Pointer to an array of characters.
 * @param[in] Number of bytes to read.
 * @return Number of bytes read from file. The return value might be less than
 * count because the end of the file is reached.
 */
int fat16_read(uint8_t handle, char *buffer, uint32_t count);

/**
 * @brief Write data to file.
 *
 * @param[in] handle Positive number returned by fat16_init.
 * @param[in] buffer Pointer to an array of characters.
 * @param[in] Number of bytes to read.
 * @return Number of bytes written to file. The return value might be less than
 * count if there is no space left.
 */
int fat16_write(uint8_t handle, char *buffer, uint32_t count);

/**
 * @brief Release the handle.
 *
 * @param[in] handle Positive number returned by fat16_open
 * @return 0 if successful, -1 otherwise
 */
int fat16_close(uint8_t handle);

/**
 * @brief Delete a file.
 *
 * The file must not be opened for reading or writing.
 * No sub directories are supported, so the path is the name of the file.
 *
 * @param[in] filename
 * @return 0 if successful, -1 otherwise
 */
int fat16_delete(char *filename);

#endif
