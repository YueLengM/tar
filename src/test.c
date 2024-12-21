
#include "tar.h"
#include "gzip.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    Tar *tar = tar_open("test.tar", TAR_MODE_WRITE);
    tar_add_file(tar, "./testfiles/test.txt", NULL);
    tar_add_file(tar, "./testfiles/test.txt", "nested_folder");
    tar_add_file(tar, "./testfiles/test2.txt", "nested_folder");
    tar_add_file(tar, "./testfiles/empty.txt", NULL);
    tar_add_file(tar, "./testfiles/test.txt", "this/is/a/very/long_____________________________________________0/file/path/in/order/to/test/prefix/split");
    tar_add_file(tar, "./testfiles/test.txt", "this/is/another/longer____________________________________________0/file/path/in/order/to/test/prefix/split");
    tar_close(tar);

    tar = tar_open("test.tar", TAR_MODE_APPEND);
    tar_add_folder_content(tar, "./testfiles", "append_test");
    tar_close(tar);

    gzip_compress("test.tar", "test.tar.gz");
    gzip_decompress("test.tar.gz", "test2.tar");

    tar = tar_open("test.tar", TAR_MODE_READ);
    tar_decompress(tar, "testout");
    tar_close(tar);
}
