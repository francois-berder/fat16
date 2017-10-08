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
