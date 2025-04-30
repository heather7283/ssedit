#pragma once

#include <cstdint>

#include "formats.hpp"

class Image {
public:
    Image(unsigned char *buf, size_t buf_size, uint32_t width, uint32_t height, Format format);
    ~Image();

    unsigned char *data;
    size_t data_size;
    uint32_t w, h;
    Format format;
};

