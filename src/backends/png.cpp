#ifdef SSEDIT_HAVE_LIBSPNG

#include <spng.h>

#include "png.hpp"
#include "log.hpp"

unsigned char *DecodePNG(const unsigned char *data, size_t data_size,
                         uint32_t *width, uint32_t *height) {
    int ret;
    unsigned char *out = nullptr;

    LogPrint(INFO, "PNG decoder: using libspng version %s", spng_version_string());

    spng_ctx *ctx = spng_ctx_new(0);

    ret = spng_set_png_buffer(ctx, data, data_size);
    if (ret != 0) {
        goto err;
    }

    size_t out_size;
    ret = spng_decoded_image_size(ctx, SPNG_FMT_RGBA8, &out_size);
    if (ret != 0) {
        goto err;
    }

    spng_ihdr ihdr;
    ret = spng_get_ihdr(ctx, &ihdr);
    if (ret != 0) {
        goto err;
    }
    LogPrint(INFO, "PNG decoder: decoding image with size %dx%d", ihdr.width, ihdr.height);
    *width = ihdr.width;
    *height = ihdr.height;

    out = new unsigned char[out_size];
    ret = spng_decode_image(ctx, out, out_size, SPNG_FMT_RGBA8, 0);
    if (ret != 0) {
        goto err;
    }

    spng_ctx_free(ctx);

    return out;

err:
    LogPrint(ERR, "PNG decoder: %s", spng_strerror(ret));

    spng_ctx_free(ctx);
    delete[] out;

    return nullptr;
}

#else // #ifdef SSEDIT_HAVE_LIBSPNG

#include "png.hpp"
#include "log.hpp"

unsigned char *DecodePNG(const unsigned char *data, size_t data_size,
                         uint32_t *width, uint32_t *height) {
    LogPrint(ERR, "PNG decoder: ssedit was compiled without PNG support, how did you get here?");

    *width = 0;
    *height = 0;
    return nullptr;
}

#endif // #ifdef SSEDIT_HAVE_LIBSPNG


