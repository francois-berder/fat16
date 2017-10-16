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
#define CLUSTER_OFFSET_FILE_ENTRY   (26)
#define SIZE_OFFSET_FILE_ENTRY      (28)

struct fat16_layout {
    uint32_t start_fat_region;              /**< offset in bytes of first FAT */
    uint32_t start_root_directory_region;   /**< offset in bytes of root directory */
    uint32_t start_data_region;             /**< offset in bytes of data region */
    uint32_t data_cluster_count;            /**< Number of clusters in data region */
};

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
    uint32_t    pos_entry;          /* Absolute position of file entry in its directory */
    uint16_t    cluster;            /* Current cluster reading/writing */
    uint16_t    offset;             /* Offset in bytes in cluster */
    uint32_t    remaining_bytes;    /* Remaining bytes to be read in bytes in the file, only used in read mode */
};

struct dir_entry {
    char        name[11];
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

void dump_dir_entry(struct dir_entry e);

/**
 * @brief Move cursor to a specific byte in data region.
 *
 * @param[in] cluster Index of the cluster
 * @param[in] offset Offset in bytes from the start of the cluster.
 * @return Position in the fat partition
 */
uint32_t move_to_data_region(uint16_t cluster, uint16_t offset);

/**
 * @brief Move cursor to an entry in the root directory.
 *
 * @param[in] entry_index Index of the entry, must not be greater than bpb.root_entry_count
 * @return Position in the fat partition
 */
uint32_t move_to_root_directory_region(uint16_t entry_index);

/**
 * @brief Move cursor to a location in the first FAT.
 *
 * @param[in] cluster Index of the cluster.
 * @return Position in the fat partition
 */
uint32_t move_to_fat_region(uint16_t cluster);

/**
 * @brief Mark a cluster in the FAT as used
 *
 * @param[out] new_cluster
 * @param[in] cluster
 * @return 0 if successful, -1 otherwise
 */
int allocate_cluster(uint16_t *new_cluster, uint16_t cluster);

/**
 * @brief Mark a cluster chain as free in the FAT
 *
 * @param[in] cluster First cluster in the chain
 */
void free_cluster_chain(uint16_t cluster);

/**
 * @brief Get next cluster
 *
 * @param[out] next_cluster
 * @param[in] cluster
 * @return 0 if successful, -1 otherwise
 */
int get_next_cluster(uint16_t *next_cluster, uint16_t cluster);

#endif
