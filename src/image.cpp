#include <cstdlib>

#include "image.hpp"

Image::Image(unsigned char *buf, size_t buf_size, uint32_t width, uint32_t height, Format format) {
    this->data = buf;
    this->data_size = buf_size;
    this->w = width;
    this->h = height;
    this->format = format;
}

Image::~Image() {
    free(this->data);
}

