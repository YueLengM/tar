#include "pathlib.h"
#include <string.h>
#include <stdio.h>

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
void join_path(const char *path1, const char *path2, char *output_buffer, rsize_t buffer_size)
{
    if (path1 == NULL)
        path1 = "";
    if (path2 == NULL)
        path2 = "";

    // 计算最终路径的最大长度：path1 + '/' + path2
    size_t len1 = strlen(path1);
    size_t len2 = strlen(path2);
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
    for (size_t i = 0; i < strlen(path); i++)
    {
        if (path[i] == '\\')
        {
            path[i] = '/';
        }
    }
}
