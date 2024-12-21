#ifndef TAR_H
#define TAR_H

// 返回值常量
#define TAR_SUCCESS 0
#define TAR_ERROR -1

// TAR 文件模式
typedef enum
{
    TAR_MODE_WRITE,  // 覆盖
    TAR_MODE_APPEND, // 追加
    TAR_MODE_READ    // 只读
} TarMode;

typedef struct Tar Tar;

/**
 * @brief 打开 tar 文件
 *
 * 该函数打开一个 tar 文件，并返回一个指向 Tar 结构体的指针。如果打开失败，则返回 NULL。
 *
 * @param filename 要打开的 tar 文件的名称。
 * @param mode 打开 tar 文件的模式。
 *
 * @return 如果成功打开 tar 文件，则返回指向 Tar 结构体的指针；否则返回 NULL。
 */
Tar *tar_open(const char *filename, TarMode mode);

/**
 * @brief 关闭 tar 文件
 *
 * 该函数关闭一个 tar 文件，并释放相关资源。如果关闭失败，则返回 TAR_ERROR。
 *
 * @param tar 指向 Tar 结构体的指针，表示要关闭的 tar 文件。
 *
 * @return 如果成功关闭 tar 文件，则返回 TAR_SUCCESS；否则返回 TAR_ERROR。
 */
int tar_close(Tar *tar);

/**
 * @brief 创建空文件夹
 *
 * 该函数在 tar 文件中创建一个空文件夹。如果创建失败，则返回 TAR_ERROR。
 *
 * @param tar 指向 Tar 结构体的指针，表示要创建文件夹的 tar 文件。
 * @param dirname 要创建的文件夹的名称。
 *
 * @return 如果成功创建文件夹，则返回 TAR_SUCCESS；否则返回 TAR_ERROR。
 */
int tar_create_folder(Tar *tar, const char *dirname);

/**
 * @brief 向 tar 文件中添加文件
 *
 * 该函数将指定路径的文件添加到 tar 文件中。如果提供了前缀路径，则文件名将被前缀路径和原始文件名拼接。
 *
 * @param tar 指向 Tar 结构体的指针。
 * @param path 要添加的文件的路径。
 * @param prefix_path 可选的前缀路径，用于拼接文件名。如果为 NULL，则不使用前缀路径。
 *
 * @return 如果成功添加文件，则返回 TAR_SUCCESS；否则返回 TAR_ERROR。
 */
int tar_add_file(Tar *tar, const char *path, const char *prefix_path);

/**
 * @brief 向 tar 文件中添加文件夹
 *
 * 该函数将指定路径的文件夹及其内容添加到 tar 文件中。如果提供了前缀路径，则文件名将被前缀路径和原始文件名拼接。
 *
 * @param tar 指向 Tar 结构体的指针。
 * @param path 要添加的文件夹的路径。
 * @param prefix_path 可选的前缀路径，用于拼接文件名。如果为 NULL，则不使用前缀路径。
 *
 * @return 如果成功添加文件夹，则返回 TAR_SUCCESS；否则返回 TAR_ERROR。
 */
int tar_add_folder(Tar *tar, const char *path, const char *prefix_path);

/**
 * @brief 向 tar 文件中添加文件夹的内容
 *
 * 作用同 tar_add_folder 但文件夹本身不占用一层。
 *
 * @param tar 指向 Tar 结构体的指针。
 * @param path 要添加的文件夹的路径。
 * @param prefix_path 可选的前缀路径，用于拼接文件名。如果为 NULL，则不使用前缀路径。
 *
 * @return 如果成功添加文件夹，则返回 TAR_SUCCESS；否则返回 TAR_ERROR。
 */
int tar_add_folder_content(Tar *tar, const char *path, const char *prefix_path);

/**
 * @brief 解压 tar 文件
 *
 * @param tar 指向 Tar 结构体的指针。
 * @param path 要解压到的文件夹路径。
 *
 * @return 如果成功解压，则返回 TAR_SUCCESS；否则返回 TAR_ERROR。
 */
int tar_decompress(Tar *tar, const char *path);

#endif // TAR_H
