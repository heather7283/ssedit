#pragma once

#include <cstddef>
#include <cstdint>

unsigned char *DecodePNG(const unsigned char *data, size_t data_size,
                         uint32_t *width, uint32_t *height);

void EncodePNGToFile(const char *filename,
                     const unsigned char *data, size_t data_size,
                     uint32_t width, uint32_t height);

