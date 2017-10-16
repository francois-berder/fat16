#include <string.h>
#include "debug.h"
#include "fat16.h"
#include "fat16_priv.h"
#include "rootdir.h"

extern struct storage_dev_t dev;
extern struct fat16_layout layout;
extern struct fat16_bpb bpb;

int find_available_entry_in_root_directory(uint16_t *entry_index)
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

int create_entry_in_root_dir(uint16_t *entry_index, char *name)
{
    struct dir_entry entry;

    /* Find a location in the root directory region */
    if (find_available_entry_in_root_directory(entry_index) < 0)
        return -1;

    memcpy(entry.filename, name, sizeof(entry.filename));
    entry.attribute = 0;
    memset(entry.reserved, 0, sizeof(entry.reserved));
    memset(entry.time, 0, sizeof(entry.time));
    memset(entry.date, 0, sizeof(entry.date));
    entry.starting_cluster = 0;
    entry.size = 0;

    move_to_root_directory_region(*entry_index);
    dev.write(&entry, sizeof(struct dir_entry));
    return 0;
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

void mark_root_entry_as_available(uint16_t entry_index)
{
    uint8_t entry_marker = 0;

    if (!last_entry_in_root_directory(entry_index))
        entry_marker = ROOT_DIR_AVAILABLE_ENTRY;
    move_to_root_directory_region(entry_index);
    dev.write(&entry_marker, sizeof(entry_marker));
}

int find_root_directory_entry(uint16_t *entry_index, char *filename)
{
    uint16_t i = 0;

    move_to_root_directory_region(0);
    for (i = 0; i < bpb.root_entry_count; ++i) {
        struct dir_entry e;
        dev.read(&e, sizeof(struct dir_entry));
        dump_root_entry(e);

        /* Skip available entry */
        if ((uint8_t)(e.filename[0]) == ROOT_DIR_AVAILABLE_ENTRY)
            continue;

        /* Check if we reach end of list of root directory entries */
        if (e.filename[0] == 0)
            break;

        /* Ignore any VFAT entry */
        if ((e.attribute & ROOT_DIR_VFAT_ENTRY) == ROOT_DIR_VFAT_ENTRY)
            continue;

        if (memcmp(filename, e.filename, sizeof(e.filename)) == 0) {
            *entry_index = i;
            return 0;
        }
    }

    FAT16DBG("FAT16: File %s not found.\n", filename);
    return -1;
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
