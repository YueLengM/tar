#include "pathlib.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>
#ifdef _WIN32
#include <direct.h>
#define mkdir(x, y) _mkdir(x)
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

int safe_strlen(const char *str)
{
    if (str == NULL)
    {
        return 0;
    }

    return strlen(str);
}

const char *get_filename_from_path(const char *path)
{
    if (path == NULL)
    {
        return NULL;
    }

    const char *last_separator = strrchr(path, '/');
    if (last_separator == NULL)
    {
        return path;
    }
    else
    {
        return last_separator + 1;
    }
}

void get_dir_from_path(const char *path, char *output_buffer, rsize_t buffer_size)
{
    if (path == NULL)
    {
        return;
    }

    char *last_separator = strrchr(path, '/');
    if (last_separator == NULL)
    {
        return;
    }
    else
    {
        *last_separator = '\0';
        strncpy(output_buffer, path, buffer_size);
        output_buffer[buffer_size - 1] = '\0';
        *last_separator = '/';
    }
}

void join_path(const char *path1, const char *path2, char *output_buffer, rsize_t buffer_size)
{
    if (path1 == NULL)
        path1 = "";
    if (path2 == NULL)
        path2 = "";

    // 计算最终路径的最大长度：path1 + '/' + path2
    size_t len1 = safe_strlen(path1);
    size_t len2 = safe_strlen(path2);
    size_t total_len = len1 + len2 + 2; // +2 为了一个 '/' 和一个 '\0'

    // 检查输出缓冲区是否足够
    if (buffer_size < total_len)
    {
        // 缓冲区不足，返回
        return;
    }

    // 如果 path1 以 '/' 结尾，避免重复的 '/'
    if (len1 == 0 || (len1 > 0 && path1[len1 - 1] == '/'))
    {
        snprintf(output_buffer, buffer_size, "%s%s", path1, path2);
    }
    else
    {
        snprintf(output_buffer, buffer_size, "%s/%s", path1, path2);
    }
}

void normalize_path_separator(char *path)
{
    if (path == NULL)
    {
        return;
    }

    // 遍历路径并替换所有的 '\' 为 '/'
    for (size_t i = 0; i < safe_strlen(path); i++)
    {
        if (path[i] == '\\')
        {
            path[i] = '/';
        }
    }
}

int is_directory_exists(const char *path)
{
    return access(path, F_OK) == 0;
}

int mkdirs(const char *path)
{
    char tmp[256];
    size_t len = safe_strlen(path);

    if (len >= 256)
    {
        return -1;
    }

    strcpy(tmp, path);
    tmp[len] = '\0';

    if (tmp[len - 1] == '/' || tmp[len - 1] == '\\')
    {
        tmp[len - 1] = '\0';
    }

    for (char *p = tmp + 1; *p; ++p)
    {
        if (*p == '/' || *p == '\\')
        {
            *p = '\0';
            if (is_directory_exists(tmp))
            {
                // empty
            }
            else if (mkdir(tmp, 0755) != 0 && errno != EEXIST)
            {
                return -1;
            }
            *p = '/';
        }
    }

    // 创建最终目录
    if (is_directory_exists(tmp))
    {
        // empty
    }
    else if (mkdir(tmp, 0755) != 0 && errno != EEXIST)
    {
        return -1;
    }

    return 0;
}
