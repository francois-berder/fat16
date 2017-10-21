#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "fat16.h"
#include "fat16_priv.h"
#include "path.h"
#include "rootdir.h"
#include "subdir.h"


#define INVALID_HANDLE  (255)
#define HANDLE_COUNT    (16)        /* Must not be greater than 254 */

struct fat16_bpb bpb;

static struct file_handle handles[HANDLE_COUNT];

struct fat16_layout layout;

struct storage_dev_t dev;

static int fat16_read_bpb(void)
{
    uint8_t data;

    memset(&bpb, 0, sizeof(struct fat16_bpb));

    /* Parse boot sector */
    FAT16DBG("FAT16: #######   BPB   #######\n");
    /*
     * jump instruction on 3 bytes.
     * Either: 0xEB,0x??, 0x90
     * or: 0xE9,0x??,0x??
     */
    dev.read_byte(&data);
    if (data == 0xEB) {
        dev.read_byte(&data);
        dev.read_byte(&data);
        if (data != 0x90)
            return -INVALID_JUMP_INSTRUCTION;
    } else if (data == 0xE9) {
        dev.read_byte(&data);
        dev.read_byte(&data);
    } else {
        return -INVALID_JUMP_INSTRUCTION;
    }

    dev.read(&bpb.oem_name, 8);
    FAT16DBG("FAT16: OEM NAME: %s\n", bpb.oem_name);
    dev.read(&bpb.bytes_per_sector, 2);
    FAT16DBG("FAT16: bytes per sector: %u\n", bpb.bytes_per_sector);
    if (bpb.bytes_per_sector != 512
        && bpb.bytes_per_sector != 1024
        && bpb.bytes_per_sector != 2048
        && bpb.bytes_per_sector != 4096)
        return -INVALID_BYTES_PER_SECTOR;

    dev.read(&bpb.sectors_per_cluster, 1);
    FAT16DBG("FAT16: sectors per cluster: %u\n", bpb.sectors_per_cluster);
    if (bpb.sectors_per_cluster != 1
        && bpb.sectors_per_cluster != 2
        && bpb.sectors_per_cluster != 4
        && bpb.sectors_per_cluster != 8
        && bpb.sectors_per_cluster != 16
        && bpb.sectors_per_cluster != 32
        && bpb.sectors_per_cluster != 64
        && bpb.sectors_per_cluster != 128)
        return -INVALID_SECTOR_PER_CLUSTER;

    if (bpb.bytes_per_sector * bpb.sectors_per_cluster > MAX_BYTES_PER_CLUSTER)
        return -INVALID_BYTES_PER_CLUSTER;

    dev.read(&bpb.reversed_sector_count, 2);
    FAT16DBG("FAT16: reserved sector count: %u\n", bpb.reversed_sector_count);
    if (bpb.reversed_sector_count != 1)
        return -INVALID_RESERVED_SECTOR_COUNT;

    dev.read(&bpb.num_fats, 1);
    FAT16DBG("FAT16: num fats: %u\n", bpb.num_fats);

    dev.read(&bpb.root_entry_count, 2);
    FAT16DBG("FAT16: root entry count: %u\n", bpb.root_entry_count);
    if ((((32 * bpb.root_entry_count) / bpb.bytes_per_sector) & 0x1) != 0)
        return -INVALID_ROOT_ENTRY_COUNT;

    dev.read(&bpb.sector_count, 2);


    /* Skip media */
    dev.read_byte(&data);

    dev.read(&bpb.fat_size, 2);
    FAT16DBG("FAT16: fat size: %u\n", bpb.fat_size);

    /* Skip sector per track for int 0x13 */
    dev.read_byte(&data);
    dev.read_byte(&data);

    /* Skip number of heads for int 0x13 */
    dev.read_byte(&data);
    dev.read_byte(&data);

    /* Skip hidden sectors */
    dev.read_byte(&data);
    dev.read_byte(&data);
    dev.read_byte(&data);
    dev.read_byte(&data);

    uint32_t sector_count_32b;
    dev.read(&sector_count_32b, 4);
    if ((bpb.sector_count != 0 && sector_count_32b != 0)
        || (bpb.sector_count == 0 && sector_count_32b == 0))
        return -INVALID_SECTOR_COUNT;

    if (bpb.sector_count == 0)
        bpb.sector_count = sector_count_32b;
    FAT16DBG("FAT16: sector count: %u\n", bpb.sector_count);

    /* Skip drive number */
    dev.read_byte(&data);

    /* Skip reserved byte */
    dev.read_byte(&data);

    dev.read_byte(&data);
    if (data == 0x29) {
        dev.read(&bpb.volume_id, 4);
        FAT16DBG("FAT16: volume ID: %u\n", bpb.volume_id);

        dev.read(&bpb.label, 11);
        FAT16DBG("FAT16: label: %s\n", bpb.label);

        dev.read(bpb.fs_type, 8);
        FAT16DBG("FAT16: fs type: %s\n", bpb.fs_type);
    }

    return 0;
}

static uint8_t find_available_handle(void)
{
    uint8_t i = 0;

    for (; i < HANDLE_COUNT; ++i)
        if (handles[i].filename[0] == 0)
            return i;

    return INVALID_HANDLE;
}

/** @return True if handle is valid, false otherwise */
static bool check_handle(uint8_t handle)
{
    if (handle >= HANDLE_COUNT)
        return false;

    if (handles[handle].filename[0] == 0)
        return false;

    return true;
}

int fat16_init(struct storage_dev_t _dev)
{
    uint32_t data_sector_count, root_directory_sector_count;

    dev = _dev;
    int ret = fat16_read_bpb();

    if (ret < 0)
        return ret;

    root_directory_sector_count = (bpb.root_entry_count * 32) / bpb.bytes_per_sector;
    FAT16DBG("FAT16: root directory sector count: %u\n", root_directory_sector_count);

    /* Find number of sectors in data region */
    data_sector_count = bpb.sector_count - (bpb.reversed_sector_count + (bpb.num_fats * bpb.fat_size) + root_directory_sector_count);
    layout.data_cluster_count = data_sector_count / bpb.sectors_per_cluster;

    if (layout.data_cluster_count < 4085
        || layout.data_cluster_count >= 65525)
        return -INVALID_FAT_TYPE;

    layout.start_fat_region = bpb.reversed_sector_count * bpb.bytes_per_sector;
    layout.start_root_directory_region = layout.start_fat_region + (bpb.num_fats * bpb.fat_size) * bpb.bytes_per_sector;
    layout.start_data_region = layout.start_root_directory_region + (root_directory_sector_count * bpb.bytes_per_sector);

    FAT16DBG("FAT16: file system layout:\n");
    FAT16DBG("\tstart_fat_region=%08X\n", layout.start_fat_region);
    FAT16DBG("\tstart_root_directory_region=%08X\n", layout.start_root_directory_region);
    FAT16DBG("\tstart_data_region=%08X\n", layout.start_data_region);
    FAT16DBG("\tdata cluster count: %u\n", layout.data_cluster_count);

    /* Make sure that all handles are available */
    memset(handles, 0, sizeof(handles));

    return 0;
}

int fat16_open(const char *filepath, char mode)
{
    int i;
    char filename[11];
    uint8_t handle = INVALID_HANDLE;

    if (mode != 'r' && mode != 'w') {
        FAT16DBG("FAT16: Invalid mode.\n");
        return -1;
    }

    if (filepath == NULL) {
        FAT16DBG("FAT16: Cannot open a file with a null path string.\n");
        return -1;
    }

    handle = find_available_handle();
    if (handle == INVALID_HANDLE) {
        FAT16DBG("FAT16: No available handle found.\n");
        return -1;
    }

    if (is_in_root(filepath)) {
        if (to_short_filename(filename, filepath) < 0)
            return -1;

        if (mode == 'w') {
            /* Delete existing file */
            if (!open_file_in_root(&handles[handle], filename, mode)) {
                if (delete_file_in_root(filename) < 0)
                    return -1;
            }

            if (create_file_in_root(filename) < 0)
                return -1;
        }

        if (open_file_in_root(&handles[handle], filename, mode) < 0) {
            handles[handle].filename[0] = 0;
            return -1;
        }
    } else {
        struct file_handle dir_handle;
        if (navigate_to_subdir(&dir_handle, filename, filepath) < 0)
            return -1;

        if (mode == 'w') {
            /* Delete existing file */
            struct file_handle h = dir_handle;
            if (!open_file_in_subdir(&h, filename, mode)) {
                h = dir_handle;
                if (delete_file_in_subdir(&h, filename) < 0)
                    return -1;
            }
            h = dir_handle;
            if (create_file_in_subdir(&h, filename) < 0)
                return -1;
        }
        if (open_file_in_subdir(&dir_handle, filename, mode) < 0)
            return -1;

        handles[handle] = dir_handle;
    }

    /*
     * To avoid bugs, we restrict opening the same file several times.
     * If a file is opened in write mode, this file cannot be opened anymore.
     * If a file is opened in read mode, this file cannot be opened later in write
     * mode. Hence, a file cannot be opened several times in read mode.
     */
    for (i = 0; i < HANDLE_COUNT; ++i) {
        if (handles[i].filename[0] == 0)
            continue;

        if (i == handle)
            continue;

        if (handles[handle].pos_entry == handles[i].pos_entry) {
            if ((mode == 'r' && handles[i].mode != 'r') || mode != 'r') {
                handles[handle].filename[0] = 0;
                return -1;
            }
        }
    }

    return handle;
}

int fat16_read(uint8_t handle, void *buffer, uint32_t count)
{
    if (check_handle(handle) == false) {
        FAT16DBG("FAT16: fat16_read: Invalid handle.\n");
        return -1;
    }

    if (handles[handle].mode != 'r') {
        FAT16DBG("FAT16: fat16_read: Cannot read with handle in write mode.\n");
        return -1;
    }

    return read_from_handle(&handles[handle], buffer, count);
}

int fat16_write(uint8_t handle, const void *buffer, uint32_t count)
{
    if (check_handle(handle) == false) {
        FAT16DBG("FAT16: fat16_write: Invalid handle.\n");
        return -1;
    }

    if (handles[handle].mode != 'w') {
        FAT16DBG("FAT16: fat16_write: Cannot write with handle in read mode.\n");
        return -1;
    }

    if (buffer == NULL) {
        FAT16DBG("FAT16: fat16_write: Cannot write using null buffer.\n");
        return -1;
    }

    if (count == 0)
        return 0;

    return write_from_handle(&handles[handle], buffer, count);
}

int fat16_close(uint8_t handle)
{
    if (check_handle(handle) == false) {
        FAT16DBG("FAT16: fat16_write: Invalid handle.\n");
        return -1;
    }

    handles[handle].filename[0] = 0;
    return 0;
}

int fat16_rm(const char *filepath)
{
    char filename[11];

    if (filepath == NULL) {
        FAT16DBG("FAT16: Cannot open a file with a null path string.\n");
        return -1;
    }

    if (is_in_root(filepath)) {
        if (to_short_filename(filename, filepath) < 0)
            return -1;

        if (delete_file_in_root(filename) < 0)
            return -1;
    } else {
        struct file_handle dir_handle;
        if (navigate_to_subdir(&dir_handle, filename, filepath) < 0)
            return -1;

        if (delete_file_in_subdir(&dir_handle, filename) < 0)
            return -1;
    }

    return 0;
}

int fat16_ls(uint16_t *index, char *filename)
{
    uint8_t name_length = 0, ext_length = 0;
    char fat_filename[11];

    if (*index > bpb.root_entry_count)
        return -1;

    if (*index == bpb.root_entry_count)
        return -2;

    if (filename == NULL)
        return -1;

    fat_filename[0] = 0;
    while (fat_filename[0] == 0
           || ((uint8_t)fat_filename[0]) == AVAILABLE_DIR_ENTRY) {
        uint8_t attribute = 0;

        /*
         * If this condition is true, the end of the root directory is reached.
         * and there are no more files to be found.
         */
        if (*index >= bpb.root_entry_count)
            return -2;

        move_to_root_directory_region((*index)++);
        dev.read(fat_filename, 11);

        /* Also reading attribute to skip any vfat entry. */
        dev.read(&attribute, 1);
        if ((attribute & VFAT_DIR_ENTRY) == VFAT_DIR_ENTRY) {
            fat_filename[0] = 0;    /* Make sure that the condition of the loop
                                     * remains true.
                                     */
            continue;
        }
    }

    /*
     * Reformat filename:
     *   - Trim name
     *   - Add '.' to separate name and extension
     *   - Trim extension
     *   - Add null terminated
     */
    for (name_length = 0; name_length < 8; ++name_length) {
        char c = fat_filename[name_length];
        if (c == ' ')
            break;

        filename[name_length] = c;
    }

    filename[name_length] = '.';

    for (ext_length = 0; ext_length < 3; ++ext_length) {
        char c = fat_filename[8 + ext_length];
        if (c == ' ')
            break;
        filename[name_length + 1 + ext_length] = c;
    }

    filename[name_length + 1 + ext_length] = '\0';

    return 0;
}

int fat16_mkdir(const char *dirpath)
{
    char dirname[11];

    if (is_in_root(dirpath)) {
        if (to_short_filename(dirname, dirpath) < 0)
            return -1;

        return create_directory_in_root(dirname);
    } else {
        struct file_handle handle;

        if (navigate_to_subdir(&handle, dirname, dirpath) < 0)
            return -1;

        return create_directory_in_subdir(&handle, dirname);
    }
}

int fat16_rmdir(const char *dirpath)
{
    char dirname[11];
    struct file_handle handle, dir_handle;
    bool in_root = is_in_root(dirpath);

    if (in_root) {
        if (to_short_filename(dirname, dirpath) < 0)
            return -1;

        if (open_directory_in_root(&handle, dirname) < 0)
            return -1;
    } else {
        if (navigate_to_subdir(&dir_handle, dirname, dirpath) < 0)
            return -1;

        handle = dir_handle;
        if (open_directory_in_subdir(&handle, dirname) < 0)
            return -1;
    }

    if (!is_subdir_empty(&handle))
        return -1;

    if (in_root)
        return delete_directory_in_root(dirname);
    else
        return delete_directory_in_subdir(&dir_handle, dirname);
}
