#pragma once

#include <cstddef>

enum class Format {
    PNG,
    JPEG,
    INVALID,
};

Format MatchFormat(const unsigned char *data, size_t data_size);

const char *FormatToString(Format format);

bool CheckFormatSupport(Format format);

