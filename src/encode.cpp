#include <unordered_map>
#include <cstring>

#include "encode.hpp"
#include "log.hpp"

#include "backends/png.hpp"
#include "backends/jpeg.hpp"

typedef void(*FileEncoderFunc)(const char *filename,
                               const unsigned char *data, size_t data_size,
                               uint32_t width, uint32_t height);

static const std::unordered_map<Format, FileEncoderFunc> file_encoders = {
    {  Format::PNG, EncodePNGToFile },
    { Format::JPEG, EncodeJPEGToFile },
};

void EncodeImageToFile(const char *filename, Format format,
                       const unsigned char *data, size_t data_size,
                       uint32_t width, uint32_t height) {
    FileEncoderFunc encoder = nullptr;

    if (!CheckFormatSupport(format)) {
        LogPrint(ERR, "Decoder: format %s is not supported", FormatToString(format));
        return;
    }

    LogPrint(INFO, "Encoder: encoding image of size %dx%d into %s to %s",
             width, height, FormatToString(format), filename);

    encoder = file_encoders.find(format)->second;
    encoder(filename, data, data_size, width, height);
}

