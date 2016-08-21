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
    uint16_t fat_size;
    uint32_t volume_id;
    char label[11];
    char fs_type[8];
};

int fat16_init(void);

#endif
