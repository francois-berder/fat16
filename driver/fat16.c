#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "fat16.h"
#include "fat16_priv.h"
#include "path.h"
#include "rootdir.h"


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

static bool is_file_opened(char *filename, bool mode)
{
    uint8_t i = 0;

    for (; i < HANDLE_COUNT; ++i) {
        if (handles[i].filename[0] == 0)
            continue;

        if (memcmp(filename, handles[i].filename, sizeof(handles[i].filename)) == 0)
            if (handles[i].read_mode == mode)
                return true;
    }

    return false;
}

static uint8_t find_available_handle(void)
{
    uint8_t i = 0;

    for (; i < HANDLE_COUNT; ++i)
        if (handles[i].filename[0] == 0)
            return i;

    return INVALID_HANDLE;
}

/**
 * @brief Create a handle for reading a file.
 *
 * @param[in] handle Index to an available handle
 * @param[in] filename Name of the file in 8.3 format
 * @return handle if successful, -1 otherwise
 */
static int fat16_open_read(uint8_t handle, char *filename)
{
    uint16_t entry_index = 0;
    struct dir_entry entry;

    /* Check that it is not opened for writing operations. */
    if (is_file_opened(filename, WRITE_MODE)) {
        FAT16DBG("FAT16: Cannot read from file while writing to it.\n");
        return -1;
    }

    /* Find the file in the root directory */
    if (find_root_directory_entry(&entry_index, filename) < 0)
        return -1;

    move_to_root_directory_region(entry_index);
    dev.read(&entry, sizeof(struct dir_entry));

    /* Create handle */
    memcpy(handles[handle].filename, filename, sizeof(handles[handle].filename));
    handles[handle].read_mode = READ_MODE;
    handles[handle].pos_entry = layout.start_root_directory_region;
    handles[handle].pos_entry += entry_index * 32;
    handles[handle].cluster = entry.starting_cluster;
    handles[handle].offset = 0;
    handles[handle].remaining_bytes = entry.size;

    return handle;
}

/**
 * @brief Create a handle and an entry in the FAT.
 *
 * @param[in] handle Index to an available handle
 * @param[in] filename Name of the file in 8.3 format
 * @return handle if successful, -1 otherwise
 */
static int fat16_open_write(uint8_t handle, char *filename)
{
    uint16_t entry_index;

    /* Check that it is not opened for reading operations. */
    if (is_file_opened(filename, READ_MODE)) {
        FAT16DBG("FAT16: Cannot write to file while reading from it.\n");
        return -1;
    }

    /*
     * Check that it is not opened for writing operations.
     * For simplicity, a file can be written by only one
     * handle.
     */
    if (is_file_opened(filename, WRITE_MODE)) {
        FAT16DBG("FAT16: Cannot write to file already opened in write mode.\n");
        return -1;
    }

    /*
     * Discard any previous content.
     * Do not check return value because the file may not exist.
     */
    delete_file_in_root(filename);

    if (create_entry_in_root_dir(&entry_index, filename) < 0)
        return -1;

    /* Create a handle */
    memcpy(handles[handle].filename, filename, sizeof(handles[handle].filename));
    handles[handle].read_mode = WRITE_MODE;
    handles[handle].pos_entry = layout.start_root_directory_region;
    handles[handle].pos_entry += entry_index * 32;
    handles[handle].cluster = 0;
    handles[handle].offset = 0;
    handles[handle].remaining_bytes = 0; /* Not used in write mode */

    return handle;
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

/**
 * @brief Update the size of file in the root entry.
 *
 * @param[in] pos_entry_index Absolute position of the file entry
 * @param[in] bytes_written_count Number of bytes appended to the file. Hence
 * the new size of the file is bytes_written_count + old_size
 */
static void update_size_file(uint32_t pos_entry, uint32_t bytes_written_count)
{
    uint32_t file_size = 0;
    uint32_t pos = pos_entry;
    pos += SIZE_OFFSET_FILE_ENTRY;

    dev.seek(pos);
    dev.read(&file_size, sizeof(file_size));

    file_size += bytes_written_count;

    dev.seek(pos);
    dev.write(&file_size, sizeof(file_size));
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

int fat16_open(const char *filename, char mode)
{
    char fat_filename[11];
    uint8_t handle = INVALID_HANDLE;

    if (mode != 'r' && mode != 'w') {
        FAT16DBG("FAT16: Invalid mode.\n");
        return -1;
    }

    if (filename == NULL) {
        FAT16DBG("FAT16: Cannot open a file with a null path string.\n");
        return -1;
    }

    handle = find_available_handle();
    if (handle == INVALID_HANDLE) {
        FAT16DBG("FAT16: No available handle found.\n");
        return -1;
    }

    if (to_short_filename(fat_filename, filename) < 0)
        return -1;

    if (mode == 'r')
        return fat16_open_read(handle, fat_filename);
    else
        return fat16_open_write(handle, fat_filename);
}


int fat16_read(uint8_t handle, void *buffer, uint32_t count)
{
    uint32_t bytes_read_count = 0;
    uint8_t *bytes = (uint8_t *)buffer;

    if (check_handle(handle) == false) {
        FAT16DBG("FAT16: fat16_read: Invalid handle.\n");
        return -1;
    }

    if (handles[handle].read_mode != READ_MODE) {
        FAT16DBG("FAT16: fat16_read: Cannot read with handle in write mode.\n");
        return -1;
    }

    /* Check if we reach end of file */
    if (handles[handle].remaining_bytes == 0)
        return 0;

    move_to_data_region(handles[handle].cluster, handles[handle].offset);

    /* Read in chunk until count is 0 or end of file is reached */
    while (count > 0) {
        uint32_t chunk_length = count, bytes_remaining_in_cluster = 0;

        /* Check if we reach end of file */
        if (handles[handle].remaining_bytes == 0)
            return bytes_read_count;

        /* Check that we read within the boundary of the current cluster */
        bytes_remaining_in_cluster = bpb.sectors_per_cluster * bpb.bytes_per_sector - handles[handle].offset;
        if (chunk_length > bytes_remaining_in_cluster)
            chunk_length = bytes_remaining_in_cluster;

        /* Check that we do not read past the end of file */
        if (chunk_length > handles[handle].remaining_bytes)
            chunk_length = handles[handle].remaining_bytes;

        dev.read(&bytes[bytes_read_count], chunk_length);

        handles[handle].remaining_bytes -= chunk_length;
        handles[handle].offset += chunk_length;
        if (handles[handle].offset == bpb.sectors_per_cluster * bpb.bytes_per_sector) {
            handles[handle].offset = 0;

            /* Look for the next cluster in the FAT, unless we are already reading the last one */
            if (handles[handle].remaining_bytes != 0) {
                uint16_t next_cluster;
                if (get_next_cluster(&next_cluster, handles[handle].cluster) < 0)
                    return -1;

                handles[handle].cluster = next_cluster;

                move_to_data_region(handles[handle].cluster, handles[handle].offset);
            }
        }
        count -= chunk_length;
        bytes_read_count += chunk_length;
    }

    return bytes_read_count;
}

int fat16_write(uint8_t handle, const void *buffer, uint32_t count)
{
    uint32_t bytes_written_count = 0;
    const uint8_t *bytes = (const uint8_t *)buffer;

    if (check_handle(handle) == false) {
        FAT16DBG("FAT16: fat16_write: Invalid handle.\n");
        return -1;
    }

    if (handles[handle].read_mode != WRITE_MODE) {
        FAT16DBG("FAT16: fat16_write: Cannot write with handle in read mode.\n");
        return -1;
    }

    if (bytes == NULL) {
        FAT16DBG("FAT16: fat16_write: Cannot write using null buffer.\n");
        return -1;
    }

    if (count == 0)
        return 0;

    /* If the file is not empty, move position to the end of file */
    if (handles[handle].cluster != 0)
        move_to_data_region(handles[handle].cluster, handles[handle].offset);

    /* Write in chunk until count is 0 or no clusters can be allocated */
    while (count > 0) {
        uint32_t chunk_length = count;
        uint32_t bytes_remaining_in_cluster = bpb.sectors_per_cluster * bpb.bytes_per_sector - handles[handle].offset;

        /* Check if we need to allocate a new cluster */
        if (handles[handle].cluster == 0
            || bytes_remaining_in_cluster == 0) {
            uint16_t new_cluster = 0;
            if (allocate_cluster(&new_cluster, handles[handle].cluster) < 0)
                return -1;

            /* If the file was empty, update cluster in root directory entry */
            if (handles[handle].cluster == 0) {
                dev.seek(handles[handle].pos_entry + CLUSTER_OFFSET_FILE_ENTRY);
                dev.write(&new_cluster, sizeof(new_cluster));
            }

            handles[handle].cluster = new_cluster;
            handles[handle].offset = 0;

            move_to_data_region(new_cluster, 0);
            bytes_remaining_in_cluster = bpb.sectors_per_cluster * bpb.bytes_per_sector;
        }

        /* Check that we write within the boundary of the current cluster */
        if (chunk_length > bytes_remaining_in_cluster)
            chunk_length = bytes_remaining_in_cluster;

        dev.write(&bytes[bytes_written_count], chunk_length);

        count -= chunk_length;
        bytes_written_count += chunk_length;
        handles[handle].offset += chunk_length;
    }

    /* Update size of file in root directory entry */
    update_size_file(handles[handle].pos_entry, bytes_written_count);

    return bytes_written_count;
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

int fat16_delete(const char *filename)
{
    char fat_filename[11];

    if (filename == NULL) {
        FAT16DBG("FAT16: Cannot open a file with a null path string.\n");
        return -1;
    }

    if (to_short_filename(fat_filename, filename) < 0)
        return -1;

    if (is_file_opened(fat_filename, READ_MODE)
        || is_file_opened(fat_filename, WRITE_MODE)) {
        FAT16DBG("FAT16: Cannot delete a file currently opened.\n");
        return -1;
    }

    return delete_file_in_root(fat_filename);
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
           || ((uint8_t)fat_filename[0]) == ROOT_DIR_AVAILABLE_ENTRY) {
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
        if ((attribute & ROOT_DIR_VFAT_ENTRY) == ROOT_DIR_VFAT_ENTRY) {
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
