#ifndef FAT16_H
#define FAT16_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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
    INVALID_FAT_TYPE,
    END_OF_FILE_REACHED,
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
 * If the end of file is reached and there are no more bytes to be read, this
 * function returns -END_OF_FILE_REACHED.
 *
 * @param[in] handle Positive number returned by fat16_open.
 * @param[out] buffer Pointer to an array of characters.
 * @param[in] Number of bytes to read.
 * @return Number of bytes read from file. The return value might be less than
 * count because the end of the file is reached.
 */
int fat16_read(uint8_t handle, char *buffer, uint32_t count);

/**
 * @brief Write data to file.
 *
 * @param[in] handle Positive number returned by fat16_open.
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

/**
 * @brief Gives the name of a file in the root directory.
 *
 * By repeatedly calling this function in this manner:
 *
 * @code{.c}
 * char filename[13];
 * int i = 0;
 * while(i >= 0) {
 *     i = fat16_ls(i, filename);
 *     if (i >= 0)
 *         printf("%s\n", filename);
 * }
 * @endcode
 *
 * It is then possible to get the complete list of files in the root directory
 * without using too much memory.
 *
 * @param[in] index Index of the entry to search for a file.
 * @param[out] filename Name in format:
 * <name> + '.' + <extension> + '\0'
 * name and extension are trimmed.
 * It is assumed that filename is at least 13 bytes long. The last byte is
 * always equal to zero.
 * @return a positive number if successful, -1 if an error occurs and -2 if
 * there are no more files in the root directory.
 */
int fat16_ls(int index, char *filename);


#ifdef __cplusplus
}
#endif

#endif
