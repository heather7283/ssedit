#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <unistd.h>

#include "log.hpp"

#define READ_CHUNK_SIZE 32768

unsigned char *ReadFromFD(int fd, size_t *size) {
    unsigned char *buffer = nullptr;
    size_t capacity = 0;
    size_t total_read = 0;

    while (1) {
        if (total_read + READ_CHUNK_SIZE > capacity) {
            size_t new_capacity = capacity + READ_CHUNK_SIZE;
            unsigned char *new_buffer = (unsigned char *)realloc(buffer, new_capacity);
            if (new_buffer == NULL) {
                LogPrint(ERR, "Reader: failed to alloc memory");
                free(buffer);
                *size = 0;
                return nullptr;
            }
            buffer = new_buffer;
            capacity = new_capacity;
        }

        ssize_t bytes_read = read(fd, buffer + total_read, READ_CHUNK_SIZE);
        if (bytes_read < 0) {
                LogPrint(ERR, "Reader: read failed (%s)", strerror(errno));
            free(buffer);
            *size = 0;
            return nullptr;
        } else if (bytes_read == 0) {
            // EOF
            break;
        }
        total_read += bytes_read;
    }

    if (total_read < capacity) {
        unsigned char *shrunk_buffer = (unsigned char *)realloc(buffer, total_read);
        if (shrunk_buffer) {
            buffer = shrunk_buffer;
        }
    }

    *size = total_read;
    return buffer;
}

bool WriteToFD(int fd, const unsigned char *buf, size_t buf_size) {
    size_t total_written = 0;

    while (total_written < buf_size) {
        ssize_t bytes_written = write(fd, buf + total_written, buf_size - total_written);
        if (bytes_written < 0) {
            LogPrint(ERR, "Writer: failed to write (%s)", strerror(errno));
            return false;
        }
        total_written += bytes_written;
    }

    return true;
}

