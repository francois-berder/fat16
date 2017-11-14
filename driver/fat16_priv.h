/*
 * Copyright (C) 2017  Francois Berder <fberder@outlook.fr>
 *
 * This file is part of fat16.
 *
 * fat16 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * fat16 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with fat16.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef __FAT16_PRIV_H__
#define __FAT16_PRIV_H__

#include <stdbool.h>
#include <stdint.h>

#define FIRST_CLUSTER_INDEX_IN_FAT     (3)
#define MAX_BYTES_PER_CLUSTER           (32768LU)
#define VFAT_DIR_ENTRY                  (0x0F)
#define AVAILABLE_DIR_ENTRY             (0xE5)


struct fat16_layout {
    uint32_t offset;                        /**< offset in bytes of the FAT16 partition */
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

struct entry_handle {
    char        mode;               /**< 'r' read from file, 'w' write to file, 'a' append to file */
    uint32_t    pos_entry;          /**< Absolute position of file entry in its directory */
    uint16_t    cluster;            /**< Current cluster reading/writing */
    uint16_t    offset;             /**< Offset in bytes in cluster */
    uint32_t    remaining_bytes;    /**< Remaining bytes to be read in bytes in the file, only used in read mode */
};

struct __attribute__((packed)) dir_entry {
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

/**
 * @brief Print content of dir_entry
 *
 * This is just a function to help debugging the driver. If compiled in
 * release mode, this function does not print anything.
 *
 * @param[in] e
 */
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

/**
 * @brief Read bytes from file/directory using handle
 *
 * @param[in] handle
 * @param[in] buffer
 * @param[in] count
 * @return number of bytes read, -1 if an error happened
 */
int read_from_handle(struct entry_handle *handle, void *buffer, uint32_t count);

/**
 * @brief Write bytes to file/directory using handle
 *
 * @param[in] handle
 * @param[in] buffer
 * @param[in] count
 * @return number of bytes written, -1 if an error happened
 */
int write_from_handle(struct entry_handle *handle, const void *buffer, uint32_t count);

/**
 * @brief Navigate to subdirectory
 *
 * @param[out] handle
 * @param[out] entry_name 8.3 short name
 * @param[in] path
 * @return 0 if succesful, -1 otherwise
 */
int navigate_to_subdir(struct entry_handle *handle, char *entry_name, const char *path);

#endif
