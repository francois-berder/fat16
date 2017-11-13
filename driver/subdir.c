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


#include <string.h>
#include "fat16.h"
#include "fat16_priv.h"
#include "subdir.h"

extern struct storage_dev_t dev;
extern struct fat16_layout layout;
extern struct fat16_bpb bpb;

/**
 * @brief Read an entry from subdir
 *
 * This functions modifies cluster/offset of the handle
 *
 * @param[out] entry
 * @param[in|out] handle
 * @return 0 if successful, -1 otherwise
 */
static int read_entry_from_subdir(struct dir_entry *entry, struct entry_handle *handle)
{
    /*
    * Check if we reach end of cluster.
    * We assume that cluster size is a multiple of dir_entry size
    */
    if (handle->offset == bpb.sectors_per_cluster * bpb.bytes_per_sector) {
        uint16_t next_cluster;
        get_next_cluster(&next_cluster, handle->cluster);
        if (next_cluster >= 0xFFF8)
            return -1;

        move_to_data_region(next_cluster, 0);
        handle->cluster = next_cluster;
        handle->offset = 0;
    }

    move_to_data_region(handle->cluster, handle->offset);
    dev.read(entry, sizeof(struct dir_entry));
    handle->offset += sizeof(struct dir_entry);
    return 0;
}

/**
 * @brief Find an entry in the subdirectory
 *
 * @param[out] entry
 * @param[out] entry_pos Absolute position of the entry
 * @param[in] handle Directory handle
 * @param[in] name 8.3 short name
 * @return 0 if an entry with this name has been found, -1 otherwise
 */
static int find_entry_in_subdir(struct dir_entry *entry, uint32_t *entry_pos, struct entry_handle *handle, char *name)
{
    int ret = -1;
    uint32_t starting_cluster = handle->cluster;

    while (read_entry_from_subdir(entry, handle) == 0) {

        /* Skip available entry */
        if ((uint8_t)(entry->name[0]) == AVAILABLE_DIR_ENTRY)
            continue;

        /* Check if we reached end of entry list */
        if (entry->name[0] == 0)
            break;

        /* Ignore any VFAT entry */
        if ((entry->attribute & VFAT_DIR_ENTRY) == VFAT_DIR_ENTRY)
            continue;

        if (memcmp(name, entry->name, sizeof(entry->name)) == 0) {
            ret = 0;
            break;
        }
    }

    if (ret == 0 && entry_pos != NULL) {
        *entry_pos = move_to_data_region(handle->cluster, handle->offset);
        *entry_pos -= sizeof(struct dir_entry);
    }

    /* Restore state of handle */
    handle->cluster = starting_cluster;
    handle->offset = 0;

    return ret;
}

static int find_available_entry_in_subdir(uint32_t *entry_pos, struct entry_handle *handle)
{
    int ret = -1;
    struct dir_entry entry;
    uint32_t starting_cluster = handle->cluster;

    /* Check if there is some space in the entry list */
    while (read_entry_from_subdir(&entry, handle) == 0) {
        if (entry.name[0] == 0 || (uint8_t)entry.name[0] == AVAILABLE_DIR_ENTRY) {
            ret = 0;
            break;
        }
    }

    if (ret == 0 && entry_pos != NULL) {
        *entry_pos = move_to_data_region(handle->cluster, handle->offset);
        *entry_pos -= sizeof(entry);
    }

    /*
     * If there is no space in the entry list, append a dummy entry to the
     * entry list.
     */
    if (ret == 0 && entry.name[0] == 0) {
        struct dir_entry dummy_entry;

        if (handle->offset == bpb.sectors_per_cluster * bpb.bytes_per_sector) {
            uint16_t new_cluster;
            if (allocate_cluster(&new_cluster, handle->cluster) < 0)
                return -1;
            handle->cluster = new_cluster;
            handle->offset = 0;
        }
        move_to_data_region(handle->cluster, handle->offset);
        memset(&dummy_entry, 0, sizeof(dummy_entry));
        dev.write(&dummy_entry, sizeof(dummy_entry));
    }

    /* Restore previous state of handle */
    handle->cluster = starting_cluster;
    handle->offset = 0;

    return ret;
}

static bool last_entry_in_subdir(uint32_t entry_pos)
{
    uint8_t tmp;
    dev.seek(entry_pos + sizeof(struct dir_entry));
    dev.read(&tmp, sizeof(tmp));
    return tmp == 0;
}

static void mark_entry_as_available(uint32_t entry_pos)
{
    struct dir_entry entry;
    memset(&entry, 0, sizeof(entry));

    if (!last_entry_in_subdir(entry_pos))
        entry.name[0] = AVAILABLE_DIR_ENTRY;

    dev.seek(entry_pos);
    dev.write(&entry, sizeof(entry));
}

int create_file_in_subdir(struct entry_handle *handle, char *filename)
{
    struct dir_entry entry;
    uint32_t entry_pos;

    /* Do not allow muliple entries with same name */
    if (find_entry_in_subdir(&entry, NULL, handle, filename) == 0)
        return -1;

    /* Try to find an available entry in the current entry list */
    if (find_available_entry_in_subdir(&entry_pos, handle) < 0)
        return -1;

    memcpy(entry.name, filename, sizeof(entry.name));
    entry.attribute = 0;
    memset(entry.reserved, 0, sizeof(entry.reserved));
    memset(entry.time, 0, sizeof(entry.time));
    memset(entry.date, 0, sizeof(entry.date));
    entry.starting_cluster = 0;
    entry.size = 0;
    dev.seek(entry_pos);
    dev.write(&entry, sizeof(entry));

    return 0;
}

int create_directory_in_subdir(struct entry_handle *handle, char *dirname)
{
    struct dir_entry entry;
    uint32_t entry_pos;
    uint32_t parent_dir_starting_cluster = handle->cluster;

    /* Do not allow muliple entries with same name */
    if (find_entry_in_subdir(&entry, NULL, handle, dirname) == 0)
        return -1;

    /* Try to find an available entry in the current entry list */
    if (find_available_entry_in_subdir(&entry_pos, handle) < 0)
        return -1;

    memcpy(entry.name, dirname, sizeof(entry.name));
    entry.attribute = SUBDIR;
    memset(entry.reserved, 0, sizeof(entry.reserved));
    memset(entry.time, 0, sizeof(entry.time));
    memset(entry.date, 0, sizeof(entry.date));
    if (allocate_cluster(&entry.starting_cluster, 0) < 0) {
        return -1;
    }
    entry.size = 0;
    dev.seek(entry_pos);
    dev.write(&entry, sizeof(entry));

    move_to_data_region(entry.starting_cluster, 0);

    /* Create "."" entry */
    {
        struct dir_entry e;
        memset(&e, 0, sizeof(e));

        e.name[0] = '.';
        memset(&e.name[1], ' ', sizeof(e.name) - 1);
        e.starting_cluster = entry.starting_cluster;
        e.attribute = SUBDIR;
        dev.write(&e, sizeof(e));
    }

    /* Create ".."" entry */
    {
        struct dir_entry e;
        memset(&e, 0, sizeof(e));

        e.name[0] = '.';
        e.name[1] = '.';
        memset(&e.name[2], ' ', sizeof(e.name) - 2);
        e.starting_cluster = parent_dir_starting_cluster;
        e.attribute = SUBDIR;

        dev.write(&e, sizeof(e));
    }

    /* Add dummy entry to indicate end of entry list */
    {
        struct dir_entry e;
        memset(&e, 0, sizeof(e));
        dev.write(&e, sizeof(e));
    }

    return 0;
}

static int open_entry_in_subdir(struct entry_handle *handle, char *name, char mode, bool is_file)
{
    struct dir_entry entry;
    uint32_t entry_pos;

    if (find_entry_in_subdir(&entry, &entry_pos, handle, name) < 0)
        return -1;

    /* Check that we are opening a file or directory and not something else */
    if (entry.attribute & VOLUME)
        return -1;

    if (is_file && entry.attribute & SUBDIR)
        return -1;

    if ((entry.attribute & READ_ONLY) && mode != 'r')
        return -1;

    handle->mode = mode;
    handle->pos_entry = entry_pos;

    /*
     * In append mode, set the current position at the end of the file.
     * Otherwise, let's start at the beginning.
     */
    if (mode == 'a') {
        handle->cluster = entry.starting_cluster;
        uint32_t offset = entry.size;
        uint16_t next_cluster;
        get_next_cluster(&next_cluster, handle->cluster);
        while (next_cluster < 0xFFF8) {
            handle->cluster = next_cluster;
            offset -= bpb.sectors_per_cluster * bpb.bytes_per_sector;
            get_next_cluster(&next_cluster, handle->cluster);
        }
        handle->offset = (uint16_t)offset;
    } else {
        handle->cluster = entry.starting_cluster;
        handle->offset = 0;
    }
    if (mode == 'r')
        handle->remaining_bytes = entry.size;
    else
        handle->remaining_bytes = 0;

    return 0;
}

int open_file_in_subdir(struct entry_handle *handle, char *filename, char mode)
{
    return open_entry_in_subdir(handle, filename, mode, true);
}

int open_directory_in_subdir(struct entry_handle *handle, char *dirname)
{
    return open_entry_in_subdir(handle, dirname, 'r', false);
}

static int delete_entry_in_subdir(struct entry_handle *handle, char *name, bool is_file)
{
    struct dir_entry entry;
    uint32_t entry_pos;

    /* Find the entry in the directory */
    if (find_entry_in_subdir(&entry, &entry_pos, handle, name) < 0)
        return -1;

    /* Check that we are deleting an entry of the right type */
    if (entry.attribute & VOLUME)
        return -1;
    if ((is_file && (entry.attribute & SUBDIR))
    ||  (!is_file && !(entry.attribute & SUBDIR)))
        return -1;

    mark_entry_as_available(entry_pos);
    free_cluster_chain(entry.starting_cluster);

    return 0;
}

int delete_file_in_subdir(struct entry_handle *handle, char *filename)
{
    return delete_entry_in_subdir(handle, filename, true);
}

int delete_directory_in_subdir(struct entry_handle *handle, char *dirname)
{
    return delete_entry_in_subdir(handle, dirname, false);
}

bool is_subdir_empty(struct entry_handle *handle)
{
    struct dir_entry entry;
    uint32_t starting_cluster = handle->cluster;
    bool is_empty = true;

    while (read_entry_from_subdir(&entry, handle) == 0) {

        /* Ignore . and .. entries */
        if (entry.name[0] == '.' && entry.name[1] == ' ')
            continue;
        if (entry.name[0] == '.' && entry.name[1] == '.' && entry.name[2] == ' ')
            continue;

        /* Check if we reached the end of the entry list */
        if (entry.name[0] == 0)
            break;

        if ((uint8_t)entry.name[0] != AVAILABLE_DIR_ENTRY) {
            is_empty = false;
            break;
        }
    }

    /* Restore state of handle */
    handle->cluster = starting_cluster;
    handle->offset = 0;

    return is_empty;
}

int ls_in_subdir(uint32_t *index, char *name, struct entry_handle *handle)
{
    struct dir_entry entry;
    uint32_t entry_index = *index;

    while (entry_index) {
        if (read_entry_from_subdir(&entry, handle) < 0)
            return -1;

        --entry_index;
    }

    if (read_entry_from_subdir(&entry, handle) < 0)
        return -1;

    if (entry.name[0] == 0)
        return 0;

    ++*index;
    memcpy(name, entry.name, sizeof(entry.name));

    return 1;
}
