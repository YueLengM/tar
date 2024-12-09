#include "tar.h"
#include "pathlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>

// 内部常量和结构
#define TAR_BLOCK_SIZE 512
#define MAX_TAR_PATH 253
#define check(func)                \
    do                             \
    {                              \
        if ((func) != TAR_SUCCESS) \
        {                          \
            return TAR_ERROR;      \
        }                          \
    } while (0)

// tar类型
typedef enum Type
{
    TAR_TYPE_FILE = '0',  // 普通文件
    TAR_TYPE_FOLDER = '5' // 文件夹
} Type;

// tar 头结构体
typedef struct TarHeader
{
    char name[100];      // 文件名
    char _mode[8];       // 权限
    char _uid[8];        // 用户 ID
    char _gid[8];        // 组 ID
    char size[12];       // 文件大小
    char mtime[12];      // 修改时间
    char checksum[8];    // 校验和
    char type;           // 文件类型
    char _linkname[100]; // 链接文件名
    char _magic[6];      // 标识 "ustar"
    char _version[2];    // 版本号
    char _uname[32];     // 用户名
    char _gname[32];     // 组名
    char _devmajor[8];   // 主设备号
    char _devminor[8];   // 次设备号
    char prefix[155];    // 前缀路径
    char _padding[12];   // 填充字节
} TarHeader;

struct Tar
{
    FILE *file;
    TarHeader *header; // 使用结构体表示 header
};

// 内部函数声明
static void init_header(TarHeader *header);
static int set_header_name(TarHeader *header, const char *name);
static int set_header(TarHeader *header, const char *name, size_t size, time_t mtime, Type type);
static int write_eof(FILE *file);
static int seek_before_eof(FILE *file);

// 初始化 tar 头部，设置默认值
static void init_header(TarHeader *header)
{
    memset(header, 0, TAR_BLOCK_SIZE);

    strcpy_s(header->_mode, sizeof(header->_mode), "0000777");
    strcpy_s(header->_uid, sizeof(header->_uid), "0000000");
    strcpy_s(header->_gid, sizeof(header->_gid), "0000000");

    strcpy_s(header->_magic, sizeof(header->_magic), "ustar");
    memcpy(header->_version, "00", sizeof(header->_version));

    strcpy_s(header->_devmajor, sizeof(header->_devmajor), "0000000");
    strcpy_s(header->_devminor, sizeof(header->_devminor), "0000000");
}

static int set_header_name(TarHeader *header, const char *name)
{
    memset(header->name, 0, sizeof(header->name));
    memset(header->prefix, 0, sizeof(header->prefix));

    size_t len = safe_strlen(name);
    // 文件名可以直接放入 name 字段
    if (len < sizeof(header->name))
    {
        strcpy_s(header->name, len + 1, name);
        return TAR_SUCCESS;
    }

    if (len > MAX_TAR_PATH)
    {
        return TAR_ERROR;
    }

    // 文件名需要拆分到 prefix 和 name 字段
    size_t seg_start = 0;
    size_t seg_end = 0;
    size_t left = 0;
    for (; seg_end < sizeof(header->prefix) && seg_end < len; ++seg_end)
    {
        if (name[seg_end] == '/')
        {
            // 添加一层到 prefix
            strncat_s(header->prefix, sizeof(header->prefix), name + seg_start, seg_end - seg_start);
            seg_start = seg_end;

            // 判断剩余部分是否可以装入 name 字段
            left = len - seg_end - 1;
            if (left < sizeof(header->name))
            {
                strcpy_s(header->name, left + 1, name + seg_end + 1);
                return TAR_SUCCESS;
            }
        }
    }

    return TAR_ERROR;
}

// 设置文件头
static int set_header(TarHeader *header, const char *name, size_t size, time_t mtime, Type type)
{
    // 文件名处理
    check(set_header_name(header, name));

    snprintf(header->size, sizeof(header->size), "%011o", (unsigned int)size);    // 文件大小
    snprintf(header->mtime, sizeof(header->mtime), "%011o", (unsigned int)mtime); // 修改时间
    header->type = type;                                                          // 文件类型

    // 计算校验和并填充checksum字段
    memset(header->checksum, ' ', sizeof(header->checksum));
    unsigned int checksum = 0;
    size_t i;
    for (i = 0; i < TAR_BLOCK_SIZE - 12; ++i)
    {
        checksum += ((unsigned char *)header)[i];
    }
    snprintf(header->checksum, sizeof(header->checksum), "%06o", checksum);

    return TAR_SUCCESS;
}

static int seek_before_eof(FILE *file)
{
    fseek(file, 0, SEEK_END);
    size_t pos = ftell(file);

    if (pos == 0)
    {
        return TAR_SUCCESS;
    }

    if (pos < TAR_BLOCK_SIZE * 2 && pos % TAR_BLOCK_SIZE != 0)
    {
        return TAR_ERROR;
    }

    fseek(file, -TAR_BLOCK_SIZE * 2, SEEK_CUR);

    return TAR_SUCCESS;
}

static int write_eof(FILE *file)
{
    char empty_block[TAR_BLOCK_SIZE] = {0};
    if (fwrite(empty_block, 1, TAR_BLOCK_SIZE, file) != TAR_BLOCK_SIZE)
    {
        return TAR_ERROR;
    }
    if (fwrite(empty_block, 1, TAR_BLOCK_SIZE, file) != TAR_BLOCK_SIZE)
    {
        return TAR_ERROR;
    }
    return TAR_SUCCESS;
}

// tar_open 实现
Tar *tar_open(const char *filename, TarMode mode)
{
    const char *open_mode = (mode == TAR_MODE_APPEND) ? "rb+" : "wb";
    FILE *file = fopen(filename, open_mode);
    if (!file)
    {
        return NULL;
    }

    // 创建 Tar 句柄
    Tar *tar = (Tar *)malloc(sizeof(Tar));
    if (!tar)
    {
        fclose(file);
        return NULL;
    }
    tar->file = file;
    tar->header = (TarHeader *)calloc(1, TAR_BLOCK_SIZE); // 申请头部结构体内存
    if (!tar->header)
    {
        fclose(file);
        free(tar);
        return NULL;
    }

    // 初始化头部字段，设置默认值
    init_header(tar->header);

    // 如果是追加模式，调整文件指针
    if (mode == TAR_MODE_APPEND)
    {
        if (seek_before_eof(file) != TAR_SUCCESS)
        {
            tar_close(tar);
            return NULL;
        }
    }

    return tar;
}

// tar_close 实现
int tar_close(Tar *tar)
{
    if (!tar || !tar->file)
    {
        return TAR_ERROR;
    }

    // 写两个空块作为结束标记
    check(write_eof(tar->file));

    fclose(tar->file);
    tar->file = NULL;
    free(tar->header);
    tar->header = NULL;
    free(tar);
    tar = NULL;

    return TAR_SUCCESS;
}

// 创建空文件夹
int tar_create_folder(Tar *tar, const char *dirname)
{
    if (!tar || !tar->file || !dirname)
    {
        return TAR_ERROR;
    }

    check(set_header(tar->header, dirname, 0, 0, TAR_TYPE_FOLDER));
    if (fwrite(tar->header, TAR_BLOCK_SIZE, 1, tar->file) != 1)
    {
        return TAR_ERROR;
    }

    return TAR_SUCCESS;
}

int tar_add_file(Tar *tar, const char *path, const char *prefix_path)
{
    if (!tar || !tar->file || !path)
    {
        return TAR_ERROR;
    }

    char path_in_tar[MAX_TAR_PATH] = {0};

    // 获取文件修改时间
    time_t mtime;
    size_t size;
    struct stat st;
    if (stat(path, &st) != 0)
    {
        return TAR_ERROR;
    }
    mtime = st.st_mtime;
    size = st.st_size;

    // 打开源文件
    FILE *src = fopen(path, "rb");
    if (!src)
    {
        return TAR_ERROR;
    }

    const char *filename = get_filename_from_path(path);
    join_path(prefix_path, filename, path_in_tar, MAX_TAR_PATH);
    normalize_path_separator(path_in_tar);

    // 设置并写入文件头
    check(set_header(tar->header, path_in_tar, size, mtime, TAR_TYPE_FILE));
    if (fwrite(tar->header, TAR_BLOCK_SIZE, 1, tar->file) != 1)
    {
        fclose(src);
        return TAR_ERROR;
    }

    // 写入文件内容
    char buffer[TAR_BLOCK_SIZE] = {0};
    size_t read_size = 0;
    while (size > 0)
    {
        read_size = fread(buffer, 1, TAR_BLOCK_SIZE, src);
        fwrite(buffer, 1, read_size, tar->file);
        size -= read_size;
    }
    if (read_size % TAR_BLOCK_SIZE != 0)
    {
        memset(buffer, 0, TAR_BLOCK_SIZE);
        fwrite(buffer, 1, TAR_BLOCK_SIZE - read_size, tar->file);
    }

    fclose(src);

    return TAR_SUCCESS;
}

int tar_add_folder(Tar *tar, const char *path, const char *prefix_path)
{
    const char *dirname = get_filename_from_path(path);
    size_t new_size = safe_strlen(prefix_path) + safe_strlen(dirname) + 2;
    if (new_size - 1 > MAX_TAR_PATH)
    {
        return TAR_ERROR;
    }

    char *new_prefix = (char *)malloc(new_size);
    join_path(prefix_path, dirname, new_prefix, new_size);

    int ret = tar_add_folder_content(tar, path, new_prefix);
    free(new_prefix);
    return ret;
}

int tar_add_folder_content(Tar *tar, const char *path, const char *prefix_path)
{
    DIR *dir;
    dir = opendir(path);
    if (dir == NULL)
    {
        return TAR_ERROR;
    }

    struct dirent *entry;
    struct stat st;
    char *content_path = NULL;
    char is_empty_dir = 1; // 换成 BOOL TRUE
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

            is_empty_dir = 0; // FALSE
            size_t path_len = safe_strlen(path) + safe_strlen(entry->d_name) + 2;
            content_path = (char *)realloc(content_path, path_len);
            snprintf(content_path, path_len, "%s/%s", path, entry->d_name);

            if (stat(content_path, &st) != 0)
            {
                closedir(dir);
                free(content_path);
                return TAR_ERROR;
            }

            if (S_ISDIR(st.st_mode))
            {
                if (tar_add_folder(tar, content_path, prefix_path) != TAR_SUCCESS)
                {
                    closedir(dir);
                    free(content_path);
                    return TAR_ERROR;
                }
            }
            else if (S_ISREG(st.st_mode))
            {
                if (tar_add_file(tar, content_path, prefix_path) != TAR_SUCCESS)
                {
                    closedir(dir);
                    free(content_path);
                    return TAR_ERROR;
                }
            }
    }
    closedir(dir);
    free(content_path);

    if (is_empty_dir == 1)
    {
        return tar_create_folder(tar, prefix_path);
    }

    return TAR_SUCCESS;
}
