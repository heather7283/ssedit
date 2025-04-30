#pragma once

#include <cstddef>
#include <cstdint>

unsigned char *DecodePNG(const unsigned char *data, size_t data_size,
                         uint32_t *width, uint32_t *height);

unsigned char *EncodePNG(unsigned char *src_data, size_t src_data_size,
                          uint32_t src_width, uint32_t src_height, size_t *out_size);

