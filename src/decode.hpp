#pragma once

#include <cstddef>

#include "image.hpp"

Image *DecodeImage(const unsigned char *data, size_t data_size);

