#ifndef PATHLIB_H
#define PATHLIB_H

#include <stddef.h>

int safe_strlen(const char *str);
const char *get_filename_from_path(const char *path);
void get_dir_from_path(const char *path, char *output_buffer, rsize_t buffer_size);
void join_path(const char *path1, const char *path2, char *output_buffer, rsize_t buffer_size);
void normalize_path_separator(char *path);
int is_directory_exists(const char *path);
int mkdirs(const char *path);

#endif // PATHLIB_H
