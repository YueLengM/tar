#include "gzip.h"
#include <stdio.h>
#include "zlib/zlib.h"

#define BUFFER_SIZE 512
#define GZIP_ENCODING 16

// 压缩文件
int gzip_compress(const char *source_file, const char *dest_file)
{
    FILE *source = fopen(source_file, "rb");
    if (!source)
    {
        return -1;
    }

    FILE *dest = fopen(dest_file, "wb");
    if (!dest)
    {
        fclose(source);
        return -1;
    }

    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    int ret = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, MAX_WBITS | GZIP_ENCODING, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK)
    {
        fclose(source);
        fclose(dest);
        return ret;
    }

    unsigned char in[BUFFER_SIZE];
    unsigned char out[BUFFER_SIZE];
    int flush;
    size_t bytes_read;

    // 读取源文件并压缩
    do
    {
        bytes_read = fread(in, 1, sizeof(in), source);
        if (bytes_read == 0)
        {
            flush = Z_FINISH;
        }
        else
        {
            flush = Z_NO_FLUSH;
        }

        strm.avail_in = bytes_read;
        strm.next_in = in;
        do
        {
            strm.avail_out = sizeof(out);
            strm.next_out = out;
            ret = deflate(&strm, flush);
            if (ret == Z_STREAM_ERROR)
            {
                deflateEnd(&strm);
                fclose(source);
                fclose(dest);
                return ret;
            }
            size_t have = sizeof(out) - strm.avail_out;
            fwrite(out, 1, have, dest);
        } while (strm.avail_out == 0);
    } while (flush != Z_FINISH);

    deflateEnd(&strm);
    fclose(source);
    fclose(dest);

    return Z_OK;
}

// 解压文件
int gzip_decompress(const char *source_file, const char *dest_file)
{
    FILE *source = fopen(source_file, "rb");
    if (!source)
    {
        return -1;
    }

    FILE *dest = fopen(dest_file, "wb");
    if (!dest)
    {
        fclose(source);
        return -1;
    }

    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    int ret = inflateInit2(&strm, MAX_WBITS | GZIP_ENCODING);
    if (ret != Z_OK)
    {
        fclose(source);
        fclose(dest);
        return ret;
    }

    unsigned char in[BUFFER_SIZE];
    unsigned char out[BUFFER_SIZE];
    size_t bytes_read;

    // 读取压缩文件并解压
    do
    {
        bytes_read = fread(in, 1, sizeof(in), source);
        if (bytes_read == 0)
        {
            break;
        }

        strm.avail_in = bytes_read;
        strm.next_in = in;

        do
        {
            strm.avail_out = sizeof(out);
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            if (ret == Z_ERRNO)
            {
                inflateEnd(&strm);
                fclose(source);
                fclose(dest);
                return ret;
            }
            size_t have = sizeof(out) - strm.avail_out;
            fwrite(out, 1, have, dest);
        } while (strm.avail_out == 0);
    } while (1);

    inflateEnd(&strm);
    fclose(source);
    fclose(dest);

    return Z_OK;
}
