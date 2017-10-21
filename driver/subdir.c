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
static int read_entry_from_subdir(struct dir_entry *entry, struct file_handle *handle)
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
static int find_entry_in_subdir(struct dir_entry *entry, uint32_t *entry_pos, struct file_handle *handle, char *name)
{
    int ret = -1;
    uint32_t starting_cluster = handle->cluster;

    while (read_entry_from_subdir(entry, handle) == 0) {

        /* Skip available entry */
        if ((uint8_t)(entry->name[0]) == ROOT_DIR_AVAILABLE_ENTRY)
            continue;

        /* Check if we reached end of entry list */
        if (entry->name[0] == 0)
            break;

        /* Ignore any VFAT entry */
        if ((entry->attribute & ROOT_DIR_VFAT_ENTRY) == ROOT_DIR_VFAT_ENTRY)
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

static int find_available_entry_in_subdir(uint32_t *entry_pos, struct file_handle *handle)
{
    int ret = -1;
    struct dir_entry entry;
    uint32_t starting_cluster = handle->cluster;

    /* Check if there is some space in the entry list */
    while (read_entry_from_subdir(&entry, handle) == 0) {
        if (entry.name[0] == 0 || (uint8_t)entry.name[0] == ROOT_DIR_AVAILABLE_ENTRY) {
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
        entry.name[0] = ROOT_DIR_AVAILABLE_ENTRY;

    dev.seek(entry_pos);
    dev.write(&entry, sizeof(entry));
}

int create_file_in_subdir(struct file_handle *handle, char *filename)
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

int create_directory_in_subdir(struct file_handle *handle, char *dirname)
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

static int open_entry_in_subdir(struct file_handle *handle, char *name, bool read_mode, bool is_file)
{
    struct dir_entry entry;
    uint32_t entry_pos;

    if (find_entry_in_subdir(&entry, &entry_pos, handle, name) < 0)
        return -1;

    /* Check that we are opening a file or directory and not something else */
    if (entry.attribute & VOLUME || entry.attribute & SYSTEM)
        return -1;

    if (is_file && entry.attribute & SUBDIR)
        return -1;

    if ((entry.attribute & READ_ONLY) && read_mode != READ_MODE)
        return -1;

    memcpy(handle->filename, name, sizeof(handle->filename));
    handle->read_mode = read_mode;
    handle->pos_entry = entry_pos;
    handle->cluster = entry.starting_cluster;
    handle->offset = 0;
    handle->remaining_bytes = entry.size;

    return 0;
}

int open_file_in_subdir(struct file_handle *handle, char *filename, bool read_mode)
{
    return open_entry_in_subdir(handle, filename, read_mode, true);
}

int open_directory_in_subdir(struct file_handle *handle, char *dirname)
{
    return open_entry_in_subdir(handle, dirname, true, false);
}

int delete_file_in_subdir(struct file_handle *handle, char *filename)
{
    struct dir_entry entry;
    uint32_t entry_pos;

    /* Find the entry in the directory */
    if (find_entry_in_subdir(&entry, &entry_pos, handle, filename) < 0)
        return -1;

    /* Check that the entry is a file */
    if (entry.attribute & VOLUME
    ||  entry.attribute & SUBDIR
    ||  entry.attribute & SYSTEM)
        return -1;

    mark_entry_as_available(entry_pos);
    free_cluster_chain(entry.starting_cluster);

    return 0;
}
