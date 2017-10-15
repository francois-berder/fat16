#ifndef __FAT16_ROOTDIR_H__
#define __FAT16_ROOTDIR_H__

/**
 * @brief Find an unused entry in the root directory.
 *
 * @param[out] entry_index
 * @retval -1 if there is no available entry in the root directory,
 * @reval 0 if successful
 */
int find_available_entry_in_root_directory(uint16_t *entry_index);

/**
 * @brief Create an entry in the root directory
 *
 * @param[out] entry_index
 * @param[in] name 8.3 short filename
 * @retval -1 if there is no available entry in the root directory,
 * @reval 0 if successful
 */
int create_entry_in_root_dir(uint16_t *entry_index, char *name);

/**
 * @brief Indicate that a root entry is now available.
 *
 * The first byte of the entry must be 0xE5 if it is not the last
 * entry in the root directory. Otherwise, a value of 0 must be
 * written.
 *
 * @param[in] entry_index
 */
void mark_root_entry_as_available(uint16_t entry_index);

/**
 * @brief Find the index of an entry based on its name.
 *
 * @param[out] entry_index
 * @param[in] filename name of the file in 8.3 format
 * @return -1 if it cannot find the entry, 0 if successful
 */
int find_root_directory_entry(uint16_t *entry_index, char *filename);

#endif
