#include <unordered_map>
#include <cstring>

#include "encode.hpp"
#include "log.hpp"

#include "backends/png.hpp"
#include "backends/jpeg.hpp"

typedef unsigned char *(*EncoderFunc)(unsigned char *src_data, size_t src_data_size,
                                      uint32_t src_width, uint32_t src_height, size_t *out_size);

static const std::unordered_map<Format, EncoderFunc> encoders = {
    {  Format::PNG, EncodePNG },
    { Format::JPEG, EncodeJPEG },
};

unsigned char *EncodeImage(Format format, unsigned char *src_data, size_t src_data_size,
                           uint32_t src_width, uint32_t src_height, size_t *out_size) {
    EncoderFunc encoder = nullptr;

    if (!CheckFormatSupport(format)) {
        LogPrint(ERR, "Encoder: format %s is not supported", FormatToString(format));
        return nullptr;
    }

    LogPrint(INFO, "Encoder: encoding image of size %dx%d into %s",
             src_width, src_height, FormatToString(format));

    encoder = encoders.find(format)->second;
    return encoder(src_data, src_data_size, src_width, src_height, out_size);
}

