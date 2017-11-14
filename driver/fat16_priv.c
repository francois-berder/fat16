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


#include <stddef.h>
#include <stdio.h>
#include "debug.h"
#include "fat16.h"
#include "fat16_priv.h"
#include "path.h"
#include "rootdir.h"
#include "subdir.h"

extern struct storage_dev_t dev;
extern struct fat16_layout layout;
extern struct fat16_bpb bpb;

void dump_dir_entry(struct dir_entry e)
{
#ifndef NDEBUG
    FAT16DBG("FAT16: name: %s\n", e.name);
    FAT16DBG("FAT16: attribute: ");
    if (e.attribute & READ_ONLY)
        FAT16DBG("FAT16: read-only ");
    if (e.attribute & HIDDEN)
        FAT16DBG("FAT16: hidden ");
    if (e.attribute & SYSTEM)
        FAT16DBG("FAT16: system ");
    if (e.attribute & VOLUME)
        FAT16DBG("FAT16: volume ");
    if (e.attribute & SUBDIR)
        FAT16DBG("FAT16: subdir ");
    if (e.attribute & ARCHIVE)
        FAT16DBG("FAT16: archive ");
    FAT16DBG("FAT16: \n");
    FAT16DBG("FAT16: starting cluster: %u\n", e.starting_cluster);
    FAT16DBG("FAT16: size: %u\n", e.size);
#else
    (void)e;
#endif

}

uint32_t move_to_data_region(uint16_t cluster, uint16_t offset)
{
    uint32_t tmp = cluster - 2;

    tmp *= bpb.sectors_per_cluster;
    tmp *= bpb.bytes_per_sector;
    uint32_t pos = layout.start_data_region;
    pos += layout.offset;
    pos += tmp;
    pos += offset;
    FAT16DBG("FAT16: Moving to %08X\n", pos);
    dev.seek(pos);
    return pos;
}

uint32_t move_to_root_directory_region(uint16_t entry_index)
{
    uint32_t pos = layout.start_root_directory_region;
    pos += layout.offset;
    pos += entry_index * 32;
    FAT16DBG("FAT16: Moving to %08X\n", pos);
    dev.seek(pos);
    return pos;
}

uint32_t move_to_fat_region(uint16_t cluster)
{
    uint32_t pos = layout.start_fat_region;
    pos += layout.offset;
    pos += cluster * 2;
    FAT16DBG("FAT16: Moving to %08X\n", pos);
    dev.seek(pos);
    return pos;
}

int allocate_cluster(uint16_t *new_cluster, uint16_t cluster)
{
    uint16_t next_cluster = FIRST_CLUSTER_INDEX_IN_FAT;

    /*
     * Find an empty location in the FAT, skip first 3 entries in the FAT,
     * because they are reserved.
     */
    move_to_fat_region(next_cluster);
    for (; next_cluster < layout.data_cluster_count - FIRST_CLUSTER_INDEX_IN_FAT; ++next_cluster) {
        uint16_t fat_entry;
        dev.read(&fat_entry, sizeof(fat_entry));

        /* Mark it as end of file */
        if (fat_entry == 0) {
            fat_entry = 0xFFFF;
            move_to_fat_region(next_cluster);
            dev.write(&fat_entry, sizeof(fat_entry));
            break;
        }
    }

    if (next_cluster == layout.data_cluster_count) {
        FAT16DBG("FAT16: Could not find an available cluster.\n");
        return -1;
    }

    /* Update current cluster to point to next one */
    if (cluster != 0) {
        move_to_fat_region(cluster);
        dev.write(&next_cluster, sizeof(next_cluster));
    }

    *new_cluster = next_cluster;
    return 0;
}

void free_cluster_chain(uint16_t cluster)
{
    /*
     * If the file is empty, the starting cluster variable is equal to 0.
     * No need to iterate through the FAT.
     */
    if (cluster == 0)
        return;

    /* Mark all clusters in the FAT as available */
    do {
        uint16_t free_cluster = 0;
        uint16_t next_cluster;
        uint32_t pos_cluster = move_to_fat_region(cluster);
        dev.read(&next_cluster, sizeof(next_cluster));

        dev.seek(pos_cluster);
        dev.write(&free_cluster, sizeof(free_cluster));

        if (next_cluster >= 0xFFF8)
            break;
        cluster = next_cluster;
    } while (1);
}

int get_next_cluster(uint16_t *next_cluster, uint16_t cluster)
{
    move_to_fat_region(cluster);
    dev.read(next_cluster, sizeof(cluster));

    return 0;
}

int read_from_handle(struct entry_handle *handle, void *buffer, uint32_t count)
{
    uint32_t bytes_read_count = 0;
    uint8_t *bytes = (uint8_t *)buffer;

    /* Check if we reach end of file */
    if (handle->remaining_bytes == 0)
        return 0;

    /* Check that cluster is valid */
    if (handle->cluster == 0)
        return 0;

    move_to_data_region(handle->cluster, handle->offset);

    /* Read in chunk until count is 0 or end of file is reached */
    while (count > 0) {
        uint32_t chunk_length = count, bytes_remaining_in_cluster = 0;

        /* Check if we reach end of file */
        if (handle->remaining_bytes == 0)
            return bytes_read_count;

        /* Check that we read within the boundary of the current cluster */
        bytes_remaining_in_cluster = bpb.sectors_per_cluster * bpb.bytes_per_sector - handle->offset;
        if (chunk_length > bytes_remaining_in_cluster)
            chunk_length = bytes_remaining_in_cluster;

        /* Check that we do not read past the end of file */
        if (chunk_length > handle->remaining_bytes)
            chunk_length = handle->remaining_bytes;

        dev.read(&bytes[bytes_read_count], chunk_length);

        handle->remaining_bytes -= chunk_length;
        handle->offset += chunk_length;
        if (handle->offset == bpb.sectors_per_cluster * bpb.bytes_per_sector) {
            handle->offset = 0;

            /* Look for the next cluster in the FAT, unless we are already reading the last one */
            if (handle->remaining_bytes != 0) {
                uint16_t next_cluster;
                if (get_next_cluster(&next_cluster, handle->cluster) < 0)
                    return -1;

                handle->cluster = next_cluster;

                move_to_data_region(handle->cluster, handle->offset);
            }
        }
        count -= chunk_length;
        bytes_read_count += chunk_length;
    }

    return bytes_read_count;
}

static void update_size_file(uint32_t pos_entry, uint32_t bytes_written_count)
{
    uint32_t file_size = 0;
    uint32_t pos = pos_entry;
    pos += offsetof(struct dir_entry, size);

    dev.seek(pos);
    dev.read(&file_size, sizeof(file_size));

    file_size += bytes_written_count;

    dev.seek(pos);
    dev.write(&file_size, sizeof(file_size));
}

int write_from_handle(struct entry_handle *handle, const void *buffer, uint32_t count)
{
    uint32_t bytes_written_count = 0;
    const uint8_t *bytes = (const uint8_t *)buffer;

    /* If the file is not empty, move position to the end of file */
    if (handle->cluster != 0)
    move_to_data_region(handle->cluster, handle->offset);

    /* Write in chunk until count is 0 or no clusters can be allocated */
    while (count > 0) {
        uint32_t chunk_length = count;
        uint32_t bytes_remaining_in_cluster = bpb.sectors_per_cluster * bpb.bytes_per_sector - handle->offset;

        /* Check if we need to allocate a new cluster */
        if (handle->cluster == 0
            || bytes_remaining_in_cluster == 0) {
            uint16_t new_cluster = 0;
            if (allocate_cluster(&new_cluster, handle->cluster) < 0)
                return -1;

            /* If the file was empty, update cluster in root directory entry */
            if (handle->cluster == 0) {
                dev.seek(handle->pos_entry + offsetof(struct dir_entry, starting_cluster));
                dev.write(&new_cluster, sizeof(new_cluster));
            }

            handle->cluster = new_cluster;
            handle->offset = 0;

            move_to_data_region(new_cluster, 0);
            bytes_remaining_in_cluster = bpb.sectors_per_cluster * bpb.bytes_per_sector;
        }

        /* Check that we write within the boundary of the current cluster */
        if (chunk_length > bytes_remaining_in_cluster)
            chunk_length = bytes_remaining_in_cluster;

        dev.write(&bytes[bytes_written_count], chunk_length);

        count -= chunk_length;
        bytes_written_count += chunk_length;
        handle->offset += chunk_length;
    }

    /* Update size of file in root directory entry */
    update_size_file(handle->pos_entry, bytes_written_count);

    return bytes_written_count;
}

int navigate_to_subdir(struct entry_handle *handle, char *entry_name, const char *path)
{
    int ret;
    char subdir_name[13];
    uint16_t index = 0;

    ret = get_subdir(subdir_name, &index, path);
    if (ret < 0) {
        return -1;
    }

    if (to_short_filename(entry_name, subdir_name) < 0)
        return -1;

    if (open_directory_in_root(handle, entry_name) < 0)
        return -1;

    while (1) {
        ret = get_subdir(subdir_name, &index, path);
        if (to_short_filename(entry_name, &path[index]) < 0)
            return -1;

        if (ret < 0)
            break;

        open_directory_in_subdir(handle, entry_name);
    }

    return 0;
}
