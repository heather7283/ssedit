#include <unordered_map>
#include <cstring>

#include "decode.hpp"
#include "formats.hpp"
#include "log.hpp"

#include "backends/png.hpp"
#include "backends/jpeg.hpp"

typedef unsigned char *(*DecoderFunc)(const unsigned char *data, size_t data_size,
                                      uint32_t *width, uint32_t *height);

static const std::unordered_map<Format, DecoderFunc> decoders = {
    {  Format::PNG,  DecodePNG },
    { Format::JPEG, DecodeJPEG },
};

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

