#ifndef GZIP_H
#define GZIP_H

// 压缩相关内存参数 zlib/zconf.h #L262
#define MAX_MEM_LEVEL 8
#define MAX_WBITS 15

// 压缩文件
int gzip_compress(const char *source_file, const char *dest_file);

// 解压文件
int gzip_decompress(const char *source_file, const char *dest_file);

#endif // GZIP_H
