#include <unordered_map>
#include <cstring>

#include "decode.hpp"
#include "formats.hpp"
#include "log.hpp"

#include "backends/png.hpp"
#include "backends/jpeg.hpp"
#include "backends/jxl.hpp"

typedef unsigned char *(*DecoderFunc)(const unsigned char *data, size_t data_size,
                                      uint32_t *width, uint32_t *height);

static const std::unordered_map<Format, DecoderFunc> decoders = {
    {  Format::PNG,  DecodePNG },
    { Format::JPEG, DecodeJPEG },
    {  Format::JXL,  DecodeJXL },
};

Image *DecodeImage(const unsigned char *data, size_t data_size) {
    Image *image = nullptr;
    DecoderFunc decoder = nullptr;
    unsigned char *out = nullptr;
    uint32_t w = 0, h = 0;

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
    out = decoder(data, data_size, &w, &h);
    if (out == nullptr) {
        goto err;
    }

    image = new Image(out, w * h * 4, w, h, Format::RGBA);
    return image;

err:
    delete image;
    return nullptr;
}

