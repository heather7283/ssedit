#include <unordered_map>
#include <cstring>

#include "encode.hpp"
#include "log.hpp"

#include "backends/png.hpp"
#include "backends/jpeg.hpp"
#include "backends/jxl.hpp"

typedef unsigned char *(*EncoderFunc)(unsigned char *src_data, size_t src_data_size,
                                      uint32_t src_width, uint32_t src_height, size_t *out_size);

static const std::unordered_map<Format, EncoderFunc> encoders = {
    {  Format::PNG,  EncodePNG },
    { Format::JPEG, EncodeJPEG },
    {  Format::JXL,  EncodeJXL },
};

Image *EncodeImage(Image *src, Format format) {
    Image *image = nullptr;
    EncoderFunc encoder = nullptr;
    unsigned char *out_buf = nullptr;
    size_t out_size = 0;

    if (!CheckFormatSupport(format)) {
        LogPrint(ERR, "Encoder: format %s is not supported", FormatToString(format));
        return nullptr;
    }

    LogPrint(INFO, "Encoder: encoding image of size %dx%d into %s",
             src->w, src->h, FormatToString(src->format));

    encoder = encoders.find(format)->second;
    out_buf = encoder(src->data, src->data_size, src->w, src->h, &out_size);
    if (out_buf == nullptr) {
        return nullptr;
    }

    image = new Image(out_buf, out_size, src->w, src->h, format);

    return image;
}

