#include <algorithm>
#include <vector>
#include <unordered_map>
#include <cstring>

#include "features.hpp"
#include "formats.hpp"

Format MatchFormat(const unsigned char *data, size_t data_size) {
    static const std::unordered_map<Format, std::vector<unsigned char>> magics = {
        {  Format::PNG, { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A } },
        { Format::JPEG, { 0xFF, 0xD8, 0xFF                               } },
    };

    auto it = std::find_if(magics.begin(), magics.end(), [data, data_size](auto &pair) {
        const auto &magic = pair.second;
        if (magic.size() > data_size) {
            return false;
        } else if (memcmp(data, magic.data(), magic.size()) == 0) {
            return true;
        } else {
            return false;
        }
    });

    if (it == std::end(magics)) {
        return Format::INVALID;
    } else {
        return it->first;
    }
}

const char *FormatToString(Format format) {
    switch (format) {
    case Format::PNG:     return "PNG";
    case Format::JPEG:    return "JPEG";
    case Format::INVALID: return "INVALID";
    default:              return "?????";
    }
}

const char *FormatToMIME(Format format) {
    switch (format) {
    case Format::PNG:     return "image/png";
    case Format::JPEG:    return "image/jpeg";
    case Format::INVALID:
    default:              return "application/octet-stream; charset=binary";
    }
}

bool CheckFormatSupport(Format format) {
    switch (format) {
    case Format::PNG:
        return HasFeaturePNG();
    case Format::JPEG:
        return HasFeatureJPEG();
    default:
        return false;
    }
}

