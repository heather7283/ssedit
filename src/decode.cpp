#include <string>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <utility>
#include <cstring>

#include "decode.hpp"
#include "features.hpp"
#include "log.hpp"

#include "backends/png.hpp"
#include "backends/jpeg.hpp"

enum class Format {
    PNG,
    JPEG,
    INVALID,
};

typedef unsigned char *(*DecoderFunc)(const unsigned char *data, size_t data_size,
                                      uint32_t *width, uint32_t *height);

static const std::unordered_map<Format, DecoderFunc> decoders = {
    {  Format::PNG,  DecodePNG },
    { Format::JPEG, DecodeJPEG },
};

static Format MatchFormat(const unsigned char *data, size_t data_size) {
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

static const char *FormatToString(Format format) {
    switch (format) {
    case Format::PNG:     return "PNG";
    case Format::JPEG:    return "JPEG";
    case Format::INVALID: return "INVALID";
    default:              return "?????";
    }
}

static bool CheckFormatSupport(Format format) {
    switch (format) {
    case Format::PNG:
        return HasFeaturePNG();
    case Format::JPEG:
        return HasFeatureJPEG();
    case Format::INVALID:
    default:
        std::unreachable();
    }
}

unsigned char *DecodeImage(const unsigned char *data, size_t data_size,
                           uint32_t *width, uint32_t *height) {
    DecoderFunc decoder = nullptr;
    unsigned char *out = nullptr;

    Format format = MatchFormat(data, data_size);
    if (format == Format::INVALID) {
        LogPrint(ERR, "Decoder: image format not recognized");
        goto err;
    }
    LogPrint(INFO, "Decoder: detected format %s", FormatToString(format));

    if (!CheckFormatSupport(format)) {
        LogPrint(ERR, "Decoder: format %s is not supported", FormatToString(format));
        goto err;
    }

    decoder = decoders.find(format)->second;
    out = decoder(data, data_size, width, height);

    return out;

err:
    delete[] out;
    *width = 0;
    *height = 0;
    return nullptr;
}

