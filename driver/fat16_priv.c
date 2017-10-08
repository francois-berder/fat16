#include <stdio.h>
#include "debug.h"
#include "fat16.h"
#include "fat16_priv.h"

extern struct storage_dev_t dev;
extern struct fat16_layout layout;
extern struct fat16_bpb bpb;

void move_to_data_region(uint16_t cluster, uint16_t offset)
{
    uint32_t tmp = cluster - 2;

    tmp *= bpb.sectors_per_cluster;
    tmp *= bpb.bytes_per_sector;
    uint32_t pos = layout.start_data_region;
    pos += tmp;
    pos += offset;
    FAT16DBG("FAT16: Moving to %08X\n", pos);
    dev.seek(pos);
}

void move_to_root_directory_region(uint16_t entry_index)
{
    uint32_t pos = layout.start_root_directory_region;

    pos += entry_index * 32;
    FAT16DBG("FAT16: Moving to %08X\n", pos);
    dev.seek(pos);
}

void move_to_fat_region(uint16_t cluster)
{
    uint32_t pos = layout.start_fat_region;

    pos += cluster * 2;
    FAT16DBG("FAT16: Moving to %08X\n", pos);
    dev.seek(pos);
}

int allocate_cluster(uint16_t *new_cluster, uint16_t cluster)
{
    uint16_t next_cluster = FIRST_CLUSTER_INDEX_IN_FAT;

    /* Find an empty location in the FAT, skip first 3 entries in the FAT,
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
    /* If the file is empty, the starting cluster variable is equal to 0.
     * No need to iterate through the FAT. */
    if (cluster == 0)
        return;

    /* Mark all clusters in the FAT as available */
    do {
        uint16_t free_cluster = 0;
        uint16_t next_cluster;
        move_to_fat_region(cluster);
        dev.read(&next_cluster, sizeof(next_cluster));

        move_to_fat_region(cluster);

        dev.write(&free_cluster, sizeof(free_cluster));

        if (next_cluster >= 0xFFF8)
            break;
        cluster = next_cluster;
    } while (1);
}
