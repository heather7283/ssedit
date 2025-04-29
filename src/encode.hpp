#pragma once

#include <cstddef>
#include <cstdint>

#include "formats.hpp"

void EncodeImageToFile(const char *filename, Format format,
                       const unsigned char *data, size_t data_size,
                       uint32_t width, uint32_t height);

