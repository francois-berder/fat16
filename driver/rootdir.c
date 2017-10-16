#include <string.h>
#include "debug.h"
#include "fat16.h"
#include "fat16_priv.h"
#include "rootdir.h"

extern struct storage_dev_t dev;
extern struct fat16_layout layout;
extern struct fat16_bpb bpb;

static int find_available_entry_in_root_directory(uint16_t *entry_index)
{
    uint16_t i = 0;
    uint32_t pos = layout.start_root_directory_region;

    do {
        uint8_t tmp;
        dev.read(&tmp, sizeof(tmp));

        if (tmp == 0 || tmp == ROOT_DIR_AVAILABLE_ENTRY) {
            *entry_index = i;
            return 0;
        }
        ++i;
        pos += 32;
        dev.seek(pos);
    } while (i < bpb.root_entry_count);

    return -1;
}

/**
 * @brief Check if the entry is the last entry in the root directory.
 *
 * @param[in] entry_index
 * @return True if the entry is the last one.
 */
static bool last_entry_in_root_directory(uint16_t entry_index)
{
    uint8_t tmp = 0;

    if (entry_index == (bpb.root_entry_count - 1))
        return true;

    /* Check if the next entry is marked as being the end of the
     * root directory list.
     */
    move_to_root_directory_region(entry_index + 1);
    dev.read(&tmp, sizeof(tmp));
    return tmp == 0;
}

static void mark_root_entry_as_available(uint16_t entry_index)
{
    uint8_t entry_marker = 0;

    if (!last_entry_in_root_directory(entry_index))
        entry_marker = ROOT_DIR_AVAILABLE_ENTRY;
    move_to_root_directory_region(entry_index);
    dev.write(&entry_marker, sizeof(entry_marker));
}

static int find_root_directory_entry(uint16_t *entry_index, char *name)
{
    uint16_t i = 0;

    move_to_root_directory_region(0);
    for (i = 0; i < bpb.root_entry_count; ++i) {
        struct dir_entry e;
        dev.read(&e, sizeof(struct dir_entry));
        dump_root_entry(e);

        /* Skip available entry */
        if ((uint8_t)(e.name[0]) == ROOT_DIR_AVAILABLE_ENTRY)
            continue;

        /* Do not allow filename to start with a NULL character */
        if (e.name[0] == 0)
            continue;

        /* Ignore any VFAT entry */
        if ((e.attribute & ROOT_DIR_VFAT_ENTRY) == ROOT_DIR_VFAT_ENTRY)
            continue;

        if (memcmp(name, e.name, sizeof(e.name)) == 0) {
            *entry_index = i;
            return 0;
        }
    }

    FAT16DBG("FAT16: File %s not found.\n", name);
    return -1;
}

static int create_entry_in_root(char *name, uint8_t attribute)
{
    uint16_t entry_index;
    struct dir_entry entry;

    /* Find a location in the root directory region */
    if (find_available_entry_in_root_directory(&entry_index) < 0)
        return -1;

    memcpy(entry.name, name, sizeof(entry.name));
    entry.attribute = attribute;
    memset(entry.reserved, 0, sizeof(entry.reserved));
    memset(entry.time, 0, sizeof(entry.time));
    memset(entry.date, 0, sizeof(entry.date));
    entry.starting_cluster = 0;
    entry.size = 0;

    move_to_root_directory_region(entry_index);
    dev.write(&entry, sizeof(struct dir_entry));
    return 0;
}

int create_file_in_root(char *filename)
{
    return create_entry_in_root(filename, 0);
}

int create_directory_in_root(char *dirname)
{
    return create_entry_in_root(dirname, SUBDIR);
}

int open_file_in_root(struct file_handle *handle, char *filename, bool read_mode)
{
    uint16_t entry_index;
    struct dir_entry entry;

    if (find_root_directory_entry(&entry_index, filename) < 0)
        return -1;

    memcpy(handle->filename, filename, sizeof(handle->filename));
    handle->read_mode = read_mode;
    handle->pos_entry = layout.start_root_directory_region;
    handle->pos_entry += entry_index * 32;

    dev.seek(handle->pos_entry);
    dev.read(&entry, sizeof(struct dir_entry));
    handle->cluster = entry.starting_cluster;
    handle->offset = 0;
    if (read_mode == WRITE_MODE)
        handle->remaining_bytes = 0;
    else
        handle->remaining_bytes = entry.size;

    return 0;
}

int delete_file_in_root(char *filename)
{
    uint16_t entry_index = 0;
    uint32_t pos = 0;
    uint16_t starting_cluster = 0;

    /* Find the file in the root directory */
    if (find_root_directory_entry(&entry_index, filename) < 0)
        return -1;

    mark_root_entry_as_available(entry_index);

    /* Find the first cluster used by the file */
    pos = layout.start_root_directory_region;
    pos += entry_index * 32;
    pos += CLUSTER_OFFSET_FILE_ENTRY;
    FAT16DBG("FAT16: Moving to %08X\n", pos);
    dev.seek(pos);
    dev.read(&starting_cluster, sizeof(starting_cluster));
    free_cluster_chain(starting_cluster);

    return 0;
}
