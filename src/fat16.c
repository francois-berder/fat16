#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "fat16.h"
#include "hal.h"

#ifndef NDEBUG
#define LOG(...)        printf(__VA_ARGS__)
#else
#define LOG(...)
#endif

#define INVALID_HANDLE  (255)
#define HANDLE_COUNT    (16)        /* Must not be greater than 254 */
#define READ_MODE       (true)
#define WRITE_MODE      (false)

#define INDEX_FIRST_CLUSTER     (2)

struct fat16_bpb {
    char oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reversed_sector_count;
    uint8_t num_fats;
    uint16_t root_entry_count;
    uint32_t sector_count;
    uint16_t fat_size; /* in sectors */
    uint32_t volume_id;
    char label[11];
    char fs_type[8];
} bpb;

struct dir_entry {
    char filename[11];
    uint8_t attribute;
    uint8_t reserved[10];
    uint8_t time[2];
    uint8_t date[2];
    uint16_t starting_cluster;
    uint32_t size;
};

enum FILE_ATTRIBUTE
{
    READ_ONLY   = 0x01,
    HIDDEN      = 0x02,
    SYSTEM      = 0x04,
    VOLUME      = 0x08,
    SUBDIR      = 0x10,
    ARCHIVE     = 0x20
};

static struct {
    char filename[11];      /* If handle is not used, filename[0] == 0 */
    bool read_mode;         /* True if reading from file, false if writing to file */
    uint16_t cluster;       /* Current cluster reading/writing */
    uint16_t offset;        /* Offset in bytes in cluster */
    uint32_t remaining_bytes;   /* Remaining bytes to be read in bytes in the file, only used in read mode */
} handles[HANDLE_COUNT];

static uint32_t start_fat_region;       /* offset in bytes of first FAT */
static uint32_t start_root_directory_region;    /* offset in bytes of root directory */
static uint32_t start_data_region;      /* offset in bytes of data region */


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

/**
 * @brief Convert filename to 8.3 short FAT name.
 *
 * Example "hello.txt" to "HELLO   TXT"
 *
 * @param[out] fat_filename 11 long char array
 * @param[in] filename arbitrary long string
 * @return 0 if successful, -1 otherwise
 */
static int make_fat_filename(char *fat_filename, char *filename)
{
    uint8_t i = 0;
    uint8_t sep;

    /* Find position of . (marker between name and extension) */
    for (i = 0; i < 9; ++i) {
        if (filename[i] == '\0')
            return -1;
        if (filename[i] == '.') {
            sep = i;
            break;
        }
    }

    /* If it cannot find . in the first 9 characters then the name is more
     * than 8 characters long which is forbidden. */
    if (i == 9)
        return -1;

    /* Copy name */
    for (i = 0; i < sep; ++i)
        fat_filename[i] = toupper(filename[i]);

    memset(&fat_filename[sep], ' ', 8 - sep);

    /* Copy extension */
    for (i = 0; i < 3; ++i) {
        if (filename[i] == '\0')
            return -1;
        fat_filename[8+i] = toupper(filename[sep+1+i]);
    }

#ifndef NDEBUG
    for(i = 0; i < 11; ++i)
        printf("%c", fat_filename[i]);
    printf("\n");
#endif
    return 0;
}

static void dump_root_entry(struct dir_entry e)
{
    LOG("filename: %s\n", e.filename);
    LOG("attribute: ");
    if (e.attribute & READ_ONLY)
        LOG("read-only ");
    if (e.attribute & HIDDEN)
        LOG("hidden ");
    if (e.attribute & SYSTEM)
        LOG("system ");
    if (e.attribute & VOLUME)
        LOG("volume ");
    if (e.attribute & SUBDIR)
        LOG("subdir ");
    if (e.attribute & ARCHIVE)
        LOG("archive ");
    LOG("\n");
    LOG("starting cluster: %u\n", e.starting_cluster);
    LOG("size: %u\n", e.size);
}

static bool is_file_opened(char *filename, bool mode)
{
    uint8_t i = 0;
    for (; i < HANDLE_COUNT; ++i) {
        if (handles[i].filename[0] == 0)
            continue;

        if (memcmp(filename, handles[i].filename, sizeof(handles[i].filename)) == 0) {
            if (handles[i].read_mode == mode)
                return true;
        }
    }

    return false;
}

static uint8_t find_available_handle(void)
{
    uint8_t i = 0;
    for (; i < HANDLE_COUNT; ++i) {
        if (handles[i].filename[0] == 0)
            return i;
    }

    return INVALID_HANDLE;
}

static void move_to_data_region(uint16_t cluster, uint16_t offset)
{
    uint32_t pos = start_data_region;
    pos += (cluster * bpb.sectors_per_cluster) * bpb.bytes_per_sector;
    pos += offset;
    LOG("Moving to %08X\n", pos);
    hal_seek(pos);
}

/**
 * @brief Move cursor to an entry in the root directory.
 *
 * @param[in] entry_index Index of the entry, must not be greater than bpb.root_entry_count
 */
static void move_to_root_directory_region(uint16_t entry_index)
{
    uint32_t pos = start_root_directory_region;
    pos += entry_index * 32;
    LOG("Moving to %08X\n", pos);
    hal_seek(pos);
}

/**
 * @brief Move cursor to a location in the first FAT.
 *
 * @param[in] cluster Cluster index - 2.
 */
static void move_to_fat_region(uint16_t cluster)
{
    uint32_t pos = start_fat_region;
    pos += (cluster - 1) * 2;
    LOG("Moving to %08X\n", pos);
    hal_seek(pos);
}

/**
 * @brief Find the index of an entry based on its name.
 *
 * @param[in] filename name of the file in 8.3 format
 * @return -1 if it cannot find the entry, otherwise give the index (positive number)
 */
static int find_root_directory_entry(char *filename)
{
    uint16_t i = 0;
    move_to_root_directory_region(0);
    for (i = 0; i < bpb.root_entry_count; ++i) {
        struct dir_entry e;
        hal_read((uint8_t*)&e, sizeof(struct dir_entry));
#ifndef NDEBUG
        //dump_root_entry(e);
#endif

        /* Skip available entry */
        if (e.filename[0] == 0xE5)
            continue;

        /* Check if we reach end of list of root directory entries */
        if (e.filename[0] == 0) {
            LOG("File %s not found.\n", filename);
            return -1;
        }

        /* Ignore any VFAT entry */
        if ((e.attribute & 0x0F) == 0x0F)
            continue;

        if (memcmp(filename, e.filename, sizeof(e.filename)) == 0)
            return i;
    }

    LOG("File %s not found.\n", filename);
    return -1;
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
    int entry_index = 0;
    struct dir_entry entry;

    /* Check that it is not opened for writing operations. */
    if (is_file_opened(filename, WRITE_MODE)) {
        LOG("Cannot read from file while writing to it.\n");
        return -1;
    }

    /* Find the file in the root directory */
    if ((entry_index = find_root_directory_entry(filename)) < 0)
        return -1;

    move_to_root_directory_region(entry_index);
    hal_read((uint8_t*)&entry, sizeof(struct dir_entry));

    /* Create handle */
    memcpy(handles[handle].filename, filename, sizeof(handles[handle].filename));
    handles[handle].read_mode = READ_MODE;
    handles[handle].cluster = entry.starting_cluster - INDEX_FIRST_CLUSTER;
    handles[handle].offset = 0;
    handles[handle].remaining_bytes = entry.size;

    return handle;
}

static int fat16_open_write(uint8_t handle, char *filename)
{
    /* Check that it is not opened for reading operations.
    if (is_file_opened(filename, READ_MODE)) {
        LOG("Cannot write to file while reading from it.\n");
        return -1;
    }

    /* Check that it is not opened for writing operations.
     * For simplicity, a file can be written by only one
     * handle.
     *
    if (is_file_opened(filename, WRITE_MODE)) {
        LOG("Cannot write to file already opened in write mode.\n");
        return -1;
    }

    /* Discard any previous content.
     * Do not check return value because the file may not exist.
     *
    fat16_delete(filename);


    /* Find a location in the root directory region */

    /* Create an entry */


    return 0;
}

int fat16_init(void)
{
    uint32_t data_sector_count, data_cluster_count, root_directory_sector_count;
    int ret = fat16_read_bpb();
    if (ret < 0)
        return ret;

    root_directory_sector_count = (bpb.root_entry_count * 32) / bpb.bytes_per_sector;
    LOG("root directory sector count: %u\n", root_directory_sector_count);

    /* Find number of sectors in data region */
    data_sector_count = bpb.sector_count - (bpb.reversed_sector_count + (bpb.num_fats * bpb.fat_size) + root_directory_sector_count);
    LOG("data sector count: %u\n", data_sector_count);

    data_cluster_count = data_sector_count / bpb.sectors_per_cluster;
    LOG("data cluster count: %u\n", data_cluster_count);

    start_fat_region = bpb.reversed_sector_count * bpb.bytes_per_sector;
    LOG("start_fat_region=%08X\n", start_fat_region);
    start_root_directory_region = start_fat_region + (bpb.num_fats * bpb.fat_size) * bpb.bytes_per_sector;
    LOG("start_root_directory_region=%08X\n", start_root_directory_region);
    start_data_region = start_root_directory_region + (root_directory_sector_count * bpb.bytes_per_sector);
    LOG("start_data_region=%08X\n", start_data_region);

    if (data_cluster_count < 4085
    ||  data_cluster_count >= 65525)
        return -INVALID_FAT_TYPE;

    return 0;
}

int fat16_open(char *filename, char mode)
{
    char fat_filename[11];
    uint8_t handle = INVALID_HANDLE;

    if (mode != 'r' && mode != 'w') {
        LOG("Invalid mode.\n");
        return -1;
    }

    if (filename == NULL) {
        LOG("Cannot open a file with a null path string.\n");
        return -1;
    }

    handle = find_available_handle();
    if (handle == INVALID_HANDLE) {
        LOG("No available handle found.\n");
        return -1;
    }

    if (make_fat_filename(fat_filename, filename) < 0)
        return -1;

    if (mode == 'r')
        return fat16_open_read(handle, fat_filename);
    else
        return fat16_open_write(handle, fat_filename);
}

/* Return true if handle is valid, false otherwise */
static bool check_handle(uint8_t handle)
{
    if (handle >= HANDLE_COUNT)
        return false;

    if (handles[handle].filename == 0)
        return false;

    return true;
}

static uint16_t read_fat_entry(uint16_t cluster)
{
    uint16_t fat_entry = 0;
    uint32_t pos = start_fat_region;
    pos += cluster * 2;
    hal_seek(pos);
    hal_read((uint8_t*)&fat_entry, sizeof(fat_entry));

    return fat_entry;
}

int fat16_read(uint8_t handle, char *buffer, uint32_t count)
{
    uint32_t bytes_read_count = 0;

    if (check_handle(handle) == false) {
        LOG("fat16_read: Invalid handle.\n");
        return -1;
    }

    if (handles[handle].read_mode != READ_MODE) {
        LOG("fat16_read: Cannot read with handle in write mode.\n");
        return -1;
    }

    /* Check if we reach end of file */
    if (handles[handle].remaining_bytes == 0)
        return -2;

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

        hal_read((uint8_t*)buffer, chunk_length);

        handles[handle].remaining_bytes -= chunk_length;
        handles[handle].offset += chunk_length;
        if (handles[handle].offset == bpb.sectors_per_cluster * bpb.bytes_per_sector) {
            handles[handle].offset = 0;

            /* Look for the next cluster in the FAT, unless we are already reading the last one */
            if (handles[handle].remaining_bytes != 0) {
                uint16_t fat_entry = read_fat_entry(handles[handle].cluster - 1);
                /* TODO: check fat entry */

                handles[handle].cluster = fat_entry;

                move_to_data_region(handles[handle].cluster, handles[handle].offset);
            }
        }
        count -= chunk_length;
        buffer += chunk_length;
        bytes_read_count += chunk_length;
    }

    return bytes_read_count;
}

int fat16_write(uint8_t handle, char *buffer, uint32_t count)
{
    if (check_handle(handle) == false) {
        LOG("fat16_write: Invalid handle.\n");
        return -1;
    }

    if (handles[handle].read_mode != WRITE_MODE) {
        LOG("fat16_write: Cannot write with handle in read mode.\n");
        return -1;
    }

    /* Write in chunk until count is 0 or no clusters can be allocated *
    while (count > 0) {
        uint32_t chunk_length = count, bytes_remaining_in_cluster = 0;


        bytes_remaining_in_cluster = bpb.sectors_per_cluster * bpb.bytes_per_sector - handles[handle].offset;

        /* Check if we need to allocate a new cluster *
        if (bytes_remaining_in_cluster == 0) {

        }

        /* Check that we write within the boundary of the current cluster
        if (chunk_length > bytes_remaining_in_cluster)
            chunk_length = bytes_remaining_in_cluster;

        hal_write();

        count -= chunk_length;
        buffer += chunk_length;
    }*/

    return -1;
}

static bool last_entry_in_root_directory(uint16_t entry_index)
{
    uint8_t tmp = 0;

    if (entry_index  == (bpb.root_entry_count - 1))
        return true;

    /* Check if the next entry is marked as being the end of the
     * root directory list.
     */
    move_to_root_directory_region(entry_index+1);
    hal_read((uint8_t*)&tmp, sizeof(tmp));
    return tmp == 0;
}

int fat16_delete(char *filename)
{
    char fat_filename[11];
    uint8_t entry_marker = 0;
    uint16_t cluster = 0;
    int entry_index;
    struct dir_entry entry;

    if (filename == NULL) {
        LOG("Cannot open a file with a null path string.\n");
        return -1;
    }

    if (make_fat_filename(fat_filename, filename) < 0)
        return -1;

    if (is_file_opened(fat_filename, READ_MODE)
    ||  is_file_opened(fat_filename, WRITE_MODE)) {
        LOG("Cannot delete a file currently opened.\n");
        return -1;
    }

    /* Find the file in the root directory */
    if ((entry_index = find_root_directory_entry(fat_filename)) < 0)
        return -1;

    /* Find the first cluster used by the file */
    move_to_root_directory_region(entry_index);
    hal_read((uint8_t*)&entry, sizeof(struct dir_entry));
    cluster = entry.starting_cluster - INDEX_FIRST_CLUSTER;

    /* The first byte of the entry must be 0xE5 if it is not the last
     * entry in the root directory. Otherwise, a value of 0 must be
     * written.
     */
    if (!last_entry_in_root_directory(entry_index))
        entry_marker = 0xE5;
    move_to_root_directory_region(entry_index);
    hal_write(&entry_marker, sizeof(entry_marker));

    /* Mark all clusters in the FAT as available */
    do {
        uint16_t free_cluster = 0;
        uint16_t next_cluster;
        move_to_fat_region(cluster);
        hal_read((uint8_t*)&next_cluster, sizeof(next_cluster));

        move_to_fat_region(cluster);

        hal_write((uint8_t*)&free_cluster, sizeof(free_cluster));

        if (next_cluster >= 0xFFF8)
            break;
        cluster = next_cluster;
    } while(1);

    return 0;
}
