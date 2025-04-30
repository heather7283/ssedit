#pragma once

#include <cstddef>

unsigned char *ReadFromFD(int fd, size_t *size);

bool WriteToFD(int fd, const unsigned char *buf, size_t buf_size);

