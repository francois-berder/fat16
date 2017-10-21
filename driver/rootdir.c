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
    dev.seek(pos);

    do {
        uint8_t tmp;
        dev.read(&tmp, sizeof(tmp));

        if (tmp == 0 || tmp == AVAILABLE_DIR_ENTRY) {
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
    struct dir_entry entry;
    memset(&entry, 0, sizeof(entry));

    if (!last_entry_in_root_directory(entry_index))
        entry.name[0] = AVAILABLE_DIR_ENTRY;

    move_to_root_directory_region(entry_index);
    dev.write(&entry, sizeof(entry));
}

static int find_root_directory_entry(uint16_t *entry_index, char *name)
{
    uint16_t i = 0;

    move_to_root_directory_region(0);
    for (i = 0; i < bpb.root_entry_count; ++i) {
        struct dir_entry e;
        dev.read(&e, sizeof(struct dir_entry));
        dump_dir_entry(e);

        /* Skip available entry */
        if ((uint8_t)(e.name[0]) == AVAILABLE_DIR_ENTRY)
            continue;

        /* Do not allow filename to start with a NULL character */
        if (e.name[0] == 0)
            continue;

        /* Ignore any VFAT entry */
        if ((e.attribute & VFAT_DIR_ENTRY) == VFAT_DIR_ENTRY)
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

    /* Do not allow muliple entries with same name */
    if (find_root_directory_entry(&entry_index, name) == 0)
        return -1;

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
    struct dir_entry entry;
    uint16_t entry_index;
    uint32_t pos;

    if (create_entry_in_root(dirname, SUBDIR) < 0)
        return -1;

    if (find_root_directory_entry(&entry_index, dirname) < 0)
        return -1;

    move_to_root_directory_region(entry_index);
    dev.read(&entry, sizeof(entry));

    if (allocate_cluster(&entry.starting_cluster, 0) < 0)
        return -1;

    pos = move_to_root_directory_region(entry_index);
    pos += CLUSTER_OFFSET_FILE_ENTRY;
    dev.seek(pos);
    dev.write(&entry.starting_cluster, sizeof(entry.starting_cluster));


    move_to_data_region(entry.starting_cluster, 0);
    /* Create "." entry */
    {
        struct dir_entry e;
        memset(&e, 0, sizeof(e));

        e.name[0] = '.';
        memset(&e.name[1], ' ', sizeof(e.name) - 1);
        e.attribute = SUBDIR;
        e.starting_cluster = entry.starting_cluster;
        dev.write(&e, sizeof(e));
    }

    /* Create ".." entry */
    {
        struct dir_entry e;
        memset(&e, 0, sizeof(e));

        e.name[0] = '.';
        e.name[1] = '.';
        memset(&e.name[2], ' ', sizeof(e.name) - 2);
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

static int open_entry_in_root(struct file_handle *handle, char *name, bool read_mode, bool is_file)
{
    uint16_t entry_index;
    struct dir_entry entry;

    if (find_root_directory_entry(&entry_index, name) < 0)
        return -1;

    handle->pos_entry = layout.start_root_directory_region;
    handle->pos_entry += entry_index * 32;
    dev.seek(handle->pos_entry);
    dev.read(&entry, sizeof(struct dir_entry));

    /* Check that we are opening a file and not something else */
    if (entry.attribute & VOLUME
    ||  entry.attribute & SYSTEM)
        return -1;
    if (is_file && entry.attribute & SUBDIR)
        return -1;

    if (entry.attribute & READ_ONLY && read_mode != READ_MODE)
        return -1;

    memcpy(handle->filename, name, sizeof(handle->filename));
    handle->read_mode = read_mode;

    handle->cluster = entry.starting_cluster;
    handle->offset = 0;
    if (read_mode == WRITE_MODE)
        handle->remaining_bytes = 0;
    else
        handle->remaining_bytes = entry.size;

    return 0;
}

int open_file_in_root(struct file_handle *handle, char *filename, bool read_mode)
{
    return open_entry_in_root(handle, filename, read_mode, true);
}

int open_directory_in_root(struct file_handle *handle, char *dirname)
{
    return open_entry_in_root(handle, dirname, true, false);
}

int delete_file_in_root(char *filename)
{
    uint16_t entry_index = 0;
    uint32_t pos = 0;
    struct dir_entry entry;

    /* Find the entry in the root directory */
    if (find_root_directory_entry(&entry_index, filename) < 0)
        return -1;

    pos = layout.start_root_directory_region;
    pos += entry_index * 32;
    dev.seek(pos);
    dev.read(&entry, sizeof(entry));

    /* Check that the entry is a file */
    if (entry.attribute & VOLUME
    ||  entry.attribute & SUBDIR
    ||  entry.attribute & SYSTEM)
        return -1;


    mark_root_entry_as_available(entry_index);
    free_cluster_chain(entry.starting_cluster);

    return 0;
}
