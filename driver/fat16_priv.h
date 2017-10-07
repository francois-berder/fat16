#ifndef __FAT16_PRIV_H__
#define __FAT16_PRIV_H__

#include <stdbool.h>
#include <stdint.h>

#define FIRST_CLUSTER_INDEX_IN_FAT     (3)
#define MAX_BYTES_PER_CLUSTER           (32768LU)
#define ROOT_DIR_VFAT_ENTRY             (0x0F)
#define ROOT_DIR_AVAILABLE_ENTRY        (0xE5)

#define READ_MODE       (1)
#define WRITE_MODE      (0)


/* cluster is a 16bit integer stored 26 bytes after the start of a
 * root directory entry.
 */
#define CLUSTER_OFFSET_ROOT_DIR_ENTRY   (26)

struct fat16_bpb {
    char        oem_name[8];
    uint16_t    bytes_per_sector;
    uint8_t     sectors_per_cluster;
    uint16_t    reversed_sector_count;
    uint8_t     num_fats;
    uint16_t    root_entry_count;
    uint32_t    sector_count;
    uint16_t    fat_size; /* in sectors */
    uint32_t    volume_id;
    char        label[11];
    char        fs_type[8];
};

struct file_handle {
    char        filename[11];       /* If handle is not used, filename[0] == 0 */
    bool        read_mode;          /* True if reading from file, false if writing to file */
    uint16_t    entry_index;        /* Index of the entry in the root directory */
    uint16_t    cluster;            /* Current cluster reading/writing */
    uint16_t    offset;             /* Offset in bytes in cluster */
    uint32_t    remaining_bytes;    /* Remaining bytes to be read in bytes in the file, only used in read mode */
};

struct dir_entry {
    char        filename[11];
    uint8_t     attribute;
    uint8_t     reserved[10];
    uint8_t     time[2];
    uint8_t     date[2];
    uint16_t    starting_cluster;
    uint32_t    size;
};

enum FILE_ATTRIBUTE {
    READ_ONLY   = 0x01,
    HIDDEN      = 0x02,
    SYSTEM      = 0x04,
    VOLUME      = 0x08,
    SUBDIR      = 0x10,
    ARCHIVE     = 0x20
};

#endif
