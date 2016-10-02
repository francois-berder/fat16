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

struct fat16_bpb {
    char oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reversed_sector_count;
    uint8_t num_fats;
    uint16_t root_entry_count;
    uint32_t sector_count;
    uint16_t fat_size; /* in sectors */
    uint32_t volume_id;
    char label[11];
    char fs_type[8];
};

struct dir_entry {
    char filename[11];
    uint8_t attribute;
    uint8_t reserved[10];
    uint8_t time[2];
    uint8_t date[2];
    uint16_t starting_cluster;
    uint32_t size;
};

enum FILE_ATTRIBUTE
{
    READ_ONLY   = 0x01,
    HIDDEN      = 0x02,
    SYSTEM      = 0x04,
    VOLUME      = 0x08,
    SUBDIR      = 0x10,
    ARCHIVE     = 0x20
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
