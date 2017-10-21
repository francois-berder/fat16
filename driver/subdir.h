#ifndef __FAT16_SUBDIR_H__
#define __FAT16_SUBDIR_H__

/**
 * @brief Create a file in a subdirectory
 *
 * No other entries with this name must exist.
 *
 * @param[in|out] handle
 * @param[in] filename 8.3 short name
 * @return 0 if successful, -1 otherwise
 */
int create_file_in_subdir(struct file_handle *handle, char *filename);

/**
 * @brief Create a directory in a subdirectory
 *
 * @param[in|out] handle
 * @param[in] dirname 8.3 short name
 * @return 0 if successful, -1 otherwise
 */
int create_directory_in_subdir(struct file_handle *handle, char *dirname);

/**
 * @brief Open a file in a subdirectory
 *
 * No file will be created by this function. It must already exist in the
 * directory
 *
 * @param[in|out] handle
 * @param[in] filename
 * @param[in] mode
 * @return 0 if successful, -1 otherwise
 */
int open_file_in_subdir(struct file_handle *handle, char *filename, char mode);

/**
 * @brief Open a directory in a subdirectory
 *
 * No directory will be created by this function. It must already
 * exist in the directory
 *
 * @param[in|out] handle
 * @param[in] filename
 * @return 0 if successful, -1 otherwise
 */
int open_directory_in_subdir(struct file_handle *handle, char *dirname);

/**
 * @brief Delete a file in a subdirectory
 *
 * @param[in] handle
 * @param[in] filename
 * @return 0 if successful, -1 otherwise
 */
int delete_file_in_subdir(struct file_handle *handle, char *filename);

/**
 * @brief Delete a directory in a subdirectory
 *
 * @param[in] handle
 * @param[in] dirname 8.3 short name
 * @return 0 if successful, -1 otherwise
 */
int delete_directory_in_subdir(struct file_handle *handle, char *dirname);

#endif
