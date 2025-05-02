#ifdef SSEDIT_HAVE_LIBSPNG

#include <spng.h>

#include "png.hpp"
#include "log.hpp"

unsigned char *DecodePNG(const unsigned char *data, size_t data_size,
                         uint32_t *width, uint32_t *height) {
    int ret = 0;
    unsigned char *out = nullptr;

    LogPrint(INFO, "PNG decoder: using libspng version %s", spng_version_string());

    spng_ctx *ctx = spng_ctx_new(0);
    if (ctx == nullptr) {
        LogPrint(ERR, "PNG decoder: failed to create spng context");
        goto err;
    }

    ret = spng_set_png_buffer(ctx, data, data_size);
    if (ret != 0) {
        goto err;
    }

    size_t out_size;
    ret = spng_decoded_image_size(ctx, SPNG_FMT_RGB8, &out_size);
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

    out = (unsigned char *)malloc(out_size);
    ret = spng_decode_image(ctx, out, out_size, SPNG_FMT_RGB8, 0);
    if (ret != 0) {
        goto err;
    }

    spng_ctx_free(ctx);

    return out;

err:
    if (ret != 0) {
        LogPrint(ERR, "PNG decoder: %s", spng_strerror(ret));
    }

    spng_ctx_free(ctx);
    free(out);

    return nullptr;
}

unsigned char *EncodePNG(unsigned char *src_data, size_t src_data_size,
                         uint32_t src_width, uint32_t src_height, size_t *out_size) {
    int ret = 0;
    struct spng_ihdr ihdr;
    unsigned char *out_buf = nullptr;
    size_t out_buf_size = 0;

    LogPrint(INFO, "PNG encoder: using libspng version %s", spng_version_string());

    spng_ctx *ctx = spng_ctx_new(SPNG_CTX_ENCODER);
    if (ctx == nullptr) {
        LogPrint(ERR, "PNG encoder: failed to create spng context");
        goto err;
    }

    ihdr = {
        .width = src_width,
        .height = src_height,
        .bit_depth = 8,
        .color_type = SPNG_COLOR_TYPE_TRUECOLOR,
        .compression_method = 0,
        .filter_method = 0,
        .interlace_method = 0,
    };
    ret = spng_set_ihdr(ctx, &ihdr);
    if (ret != 0) {
        goto err;
    }

    ret = spng_set_option(ctx, SPNG_ENCODE_TO_BUFFER, 1);
    if (ret != 0) {
        goto err;
    }

    ret = spng_encode_image(ctx, src_data, src_data_size, SPNG_FMT_PNG, SPNG_ENCODE_FINALIZE);
    if (ret != 0) {
        goto err;
    }

    out_buf = (unsigned char *)spng_get_png_buffer(ctx, &out_buf_size, &ret);
    if (ret != 0) {
        goto err;
    }

    spng_ctx_free(ctx);

    *out_size = out_buf_size;
    return out_buf;

err:
    if (ret != 0) {
        LogPrint(ERR, "PNG encoder: %s", spng_strerror(ret));
    }

    if (ctx != nullptr) {
        spng_ctx_free(ctx);
    }
    if (out_buf != nullptr) {
        free(out_buf);
    }

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

unsigned char *EncodePNG(unsigned char *src_data, size_t src_data_size,
                          uint32_t src_width, uint32_t src_height, size_t *out_size) {
    LogPrint(ERR, "PNG encoder: ssedit was compiled without PNG support, how did you get here?");

    return nullptr;
}

#endif // #ifdef SSEDIT_HAVE_LIBSPNG


