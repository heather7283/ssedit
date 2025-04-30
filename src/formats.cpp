#include <algorithm>
#include <vector>
#include <unordered_map>
#include <cstring>

#include "features.hpp"
#include "formats.hpp"

#define STRCASEEQ(a, b) (strcasecmp((a), (b)) == 0)

Format MatchFormat(const unsigned char *data, size_t data_size) {
    static const std::unordered_map<Format, std::vector<unsigned char>> magics = {
        {  Format::PNG, { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A } },
        { Format::JPEG, { 0xFF, 0xD8, 0xFF                               } },
        {  Format::JXL, { 0xFF, 0x0A                                     } },
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

Format FormatFromString(const char *string) {
    if (STRCASEEQ(string, "PNG")) {
        return Format::PNG;
    } else if (STRCASEEQ(string, "JPG") || STRCASEEQ(string, "JPEG")) {
        return Format::JPEG;
    } else if (STRCASEEQ(string, "JPEGXL") || STRCASEEQ(string, "JXL")) {
        return Format::JXL;
    } else {
        return Format::INVALID;
    }
}

const char *FormatToString(Format format) {
    switch (format) {
    case Format::PNG:     return "PNG";
    case Format::JPEG:    return "JPEG";
    case Format::JXL:     return "JXL";
    case Format::RGBA:    return "RGBA";
    case Format::INVALID: return "INVALID";
    default:              return "?????";
    }
}

const char *FormatToMIME(Format format) {
    switch (format) {
    case Format::PNG:     return "image/png";
    case Format::JPEG:    return "image/jpeg";
    case Format::JXL:     return "image/jxl";
    default:              return "application/octet-stream; charset=binary";
    }
}

bool CheckFormatSupport(Format format) {
    switch (format) {
    case Format::PNG:
        return HasFeaturePNG();
    case Format::JPEG:
        return HasFeatureJPEG();
    case Format::JXL:
        return HasFeatureJXL();
    default:
        return false;
    }
}

