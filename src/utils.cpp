#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>

#include "log.hpp"

unsigned char *ReadFileIntoMemory(const char *path, size_t *size) {
    FILE *file = nullptr;
    unsigned char *buffer = nullptr;
    long file_size;
    size_t bytes_read;

    file = fopen(path, "rb");
    if (!file) {
        LogPrint(ERR, "File reader: failed to open %s (%s)", path, strerror(errno));
        goto err;
    }

    if (fseek(file, 0, SEEK_END)) {
        LogPrint(ERR, "File reader: failed to seek to end of file (%s)", strerror(errno));
        goto err;
    }

    file_size = ftell(file);
    if (file_size == -1) {
        LogPrint(ERR, "File reader: failed to get file size (%s)", strerror(errno));
        goto err;
    }

    rewind(file);

    buffer = new unsigned char[file_size];
    bytes_read = fread(buffer, 1, file_size, file);
    if (bytes_read != (size_t)file_size) {
        LogPrint(ERR, "File reader: failed to read (%s)", strerror(errno));
        goto err;
    }

    if (size) {
        *size = bytes_read;
    }

    fclose(file);
    return buffer;

err:
    if (file) {
        fclose(file);
    }
    delete[] buffer;
    return nullptr;
}

