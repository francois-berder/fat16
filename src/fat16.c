#include <stdio.h>
#include <string.h>
#include "fat16.h"
#include "hal.h"

#ifndef NDEBUG
#define LOG(...)        printf(__VA_ARGS__)
#else
#define LOG(...)
#endif

static struct fat16_bpb bpb;
static uint32_t root_directory_sector_count;

static int fat16_read_bpb(void)
{
    uint8_t data;

    memset(&bpb, 0, sizeof(struct fat16_bpb));

    /* Parse boot sector */
    LOG("#######   BPB   #######\n");
    /* jump instruction on 3 bytes.
     * Either: 0xEB,0x??, 0x90
     * or: 0xE9,0x??,0x??
     */
    hal_read_byte(&data);
    if (data == 0xEB) {
        hal_read_byte(&data);
        hal_read_byte(&data);
        if (data != 0x90)
            return -INVALID_JUMP_INSTRUCTION;
    } else if (data == 0xE9) {
        hal_read_byte(&data);
        hal_read_byte(&data);
    } else {
        return -INVALID_JUMP_INSTRUCTION;
    }

    hal_read((uint8_t*)&bpb.oem_name, 8);
    LOG("OEM NAME: %s\n", bpb.oem_name);
    hal_read((uint8_t*)&bpb.bytes_per_sector, 2);
    LOG("bytes per sector: %u\n", bpb.bytes_per_sector);
    if (bpb.bytes_per_sector != 512
    &&  bpb.bytes_per_sector != 1024
    &&  bpb.bytes_per_sector != 2048
    &&  bpb.bytes_per_sector != 4096)
        return -INVALID_BYTES_PER_SECTOR;

    hal_read(&bpb.sectors_per_cluster, 1);
    LOG("sectors per cluster: %u\n", bpb.sectors_per_cluster);
    if (bpb.sectors_per_cluster != 1
    &&  bpb.sectors_per_cluster != 2
    &&  bpb.sectors_per_cluster != 4
    &&  bpb.sectors_per_cluster != 8
    &&  bpb.sectors_per_cluster != 16
    &&  bpb.sectors_per_cluster != 32
    &&  bpb.sectors_per_cluster != 64
    &&  bpb.sectors_per_cluster != 128)
        return -INVALID_SECTOR_PER_CLUSTER;

    if (bpb.bytes_per_sector * bpb.sectors_per_cluster > 32*1024)
        return -INVALID_BYTES_PER_CLUSTER;

    hal_read((uint8_t*)&bpb.reversed_sector_count, 2);
    LOG("reserved sector count: %u\n", bpb.reversed_sector_count);
    if (bpb.reversed_sector_count != 1)
        return -INVALID_RESERVED_SECTOR_COUNT;

    hal_read(&bpb.num_fats, 1);
    LOG("num fats: %u\n", bpb.num_fats);

    hal_read((uint8_t*)&bpb.root_entry_count, 2);
    LOG("root entry count: %u\n", bpb.root_entry_count);
    if ((((32 * bpb.root_entry_count) / bpb.bytes_per_sector) & 0x1) != 0)
        return -INVALID_ROOT_ENTRY_COUNT;

    hal_read((uint8_t*)&bpb.sector_count, 2);


    /* Skip media */
    hal_read_byte(&data);

    hal_read((uint8_t*)&bpb.fat_size, 2);
    LOG("fat size: %u\n", bpb.fat_size);

    /* Skip sector per track for int 0x13 */
    hal_read_byte(&data);
    hal_read_byte(&data);

    /* Skip number of heads for int 0x13 */
    hal_read_byte(&data);
    hal_read_byte(&data);

    /* Skip hidden sectors */
    hal_read_byte(&data);
    hal_read_byte(&data);
    hal_read_byte(&data);
    hal_read_byte(&data);

    uint32_t sector_count_32b;
    hal_read((uint8_t*)&sector_count_32b, 4);
    if ((bpb.sector_count != 0 && sector_count_32b != 0)
    ||  (bpb.sector_count == 0 && sector_count_32b == 0))
        return -INVALID_SECTOR_COUNT;

    if (bpb.sector_count == 0)
        bpb.sector_count = sector_count_32b;
    LOG("sector count: %u\n", bpb.sector_count);

    /* Skip drive number */
    hal_read_byte(&data);

    /* Skip reserved byte */
    hal_read_byte(&data);

    hal_read_byte(&data);
    if (data == 0x29) {
        hal_read((uint8_t*)&bpb.volume_id, 4);
        LOG("volume ID: %u\n", bpb.volume_id);

        hal_read((uint8_t*)&bpb.label, 11);
        LOG("label: %s\n", bpb.label);

        hal_read((uint8_t*)bpb.fs_type, 8);
        LOG("fs type: %s\n", bpb.fs_type);
    }

    return 0;
}

int fat16_init(void)
{
    uint32_t data_sector_count, data_cluster_count;
    int ret = fat16_read_bpb();
    if (ret < 0)
        return ret;

    root_directory_sector_count = ((bpb.root_entry_count * 32) + (bpb.bytes_per_sector - 1)) / bpb.bytes_per_sector;
    LOG("root directory sector count: %u\n", root_directory_sector_count);

    /* Find number of sectors in data region */
    data_sector_count = bpb.sector_count - (bpb.reversed_sector_count + (bpb.num_fats * bpb.fat_size) + root_directory_sector_count);
    LOG("data sector count: %u\n", data_sector_count);

    data_cluster_count = data_sector_count / bpb.sectors_per_cluster;
    LOG("data cluster count: %u\n", data_cluster_count);


    if (data_cluster_count < 4085
    ||  data_cluster_count >= 65525)
        return -INVALID_FAT_TYPE;

    return 0;
}
