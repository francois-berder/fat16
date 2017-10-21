#ifndef __FAT16_ROOTDIR_H__
#define __FAT16_ROOTDIR_H__

/**
 * @brief Create a file in the root directory
 *
 * @param[in] filename 8.3 short name
 * @retval -1 if there is no available entry in the root directory,
 * @reval 0 if successful
 */
int create_file_in_root(char *filename);

/**
 * @brief Create a directory in the root directory
 *
 * @param[in] dirname 8.3 short filename
 * @retval -1 if there is no available entry in the root directory,
 * @reval 0 if successful
 */
int create_directory_in_root(char *dirname);

/**
 * @brief Open a file located in the root directory
 *
 * @param[out] handle
 * @param[in] filename 8.3 short name
 * @param[in] mode
 * @return 0 if successful, -1 otherwise
 */
int open_file_in_root(struct file_handle *handle, char *filename, char mode);

/**
 * @brief Open a directory located in the root directory
 *
 * @param[out] handle
 * @param[in] dirname 8.3 short filename
 * @return 0 if successful, -1 otherwise
 */
int open_directory_in_root(struct file_handle *handle, char *dirname);

/**
 * @brief Delete a file.
 *
 * Remove the entry from the and mark all clusters used by this file as
 * available. It does not clear the data region.
 *
 * @param[in] filename 8.3 short name
 * @return 0 if successful, -1 otherwise
 */
int delete_file_in_root(char *filename);

/**
 * @brief Delete a directory
 *
 * @param dirname 8.3 short name
 * @return 0 if successful, -1 otherwise
 */
int delete_directory_in_root(char *dirname);

#endif
