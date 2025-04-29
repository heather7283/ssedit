#ifdef SSEDIT_HAVE_LIBTURBOJPEG

#include <turbojpeg.h>

#include "jpeg.hpp"
#include "log.hpp"

unsigned char *DecodeJPEG(const unsigned char *data, size_t data_size,
                          uint32_t *width, uint32_t *height) {
    tjhandle tj_instance = nullptr;
    unsigned char *out_buf = nullptr;
    int pitch, jpeg_w, jpeg_h, jpeg_subsamp, jpeg_colorspace;
    const int pixel_format = TJPF_RGBA;

    LogPrint(INFO, "JPEG decoder: using libturbojpeg");

    tj_instance = tjInitDecompress();
    if (tj_instance == nullptr) {
        LogPrint(ERR, "JPEG decoder: tjInitDecompress() failed: %s", tjGetErrorStr());
        goto err;
    }

    if (tjDecompressHeader3(tj_instance, data, data_size,
                            &jpeg_w, &jpeg_h, &jpeg_subsamp, &jpeg_colorspace) != 0) {
        LogPrint(ERR, "JPEG decoder: tjDecompressHeader3() failed: %s", tjGetErrorStr());
        goto err;
    }
    LogPrint(INFO, "JPEG decoder: decoding image with size %dx%d", jpeg_w, jpeg_h);

    pitch = jpeg_w * tjPixelSize[pixel_format];
    out_buf = new unsigned char[jpeg_h * pitch];

    if (tjDecompress2(tj_instance, data, data_size,
                      out_buf, jpeg_w, pitch, jpeg_h, pixel_format, 0) != 0) {
        LogPrint(ERR, "JPEG decoder: tjDecompress2() failed: %s", tjGetErrorStr());
        goto err;
    }

    *width = (uint32_t)jpeg_w;
    *height = (uint32_t)jpeg_h;

    tjDestroy(tj_instance);

    return out_buf;

err:
    if (tj_instance != nullptr) {
        tjDestroy(tj_instance);
    }
    delete[] out_buf;
    return nullptr;
}

#else // #ifdef SSEDIT_HAVE_LIBTURBOJPEG

#include "jpeg.hpp"
#include "log.hpp"

unsigned char *DecodeJPEG(const unsigned char *data, size_t data_size,
                          uint32_t *width, uint32_t *height) {
    LogPrint(ERR, "JPEG decoder: ssedit was compiled without JPEG support, how did you get here?");

    *width = 0;
    *height = 0;
    return nullptr;
}

#endif // #ifdef SSEDIT_HAVE_LIBTURBOJPEG

