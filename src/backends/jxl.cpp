#ifdef SSEDIT_HAVE_LIBJXL

#include <jxl/codestream_header.h>
#include <jxl/decode.h>
#include <jxl/decode_cxx.h>
#include <jxl/resizable_parallel_runner.h>
#include <jxl/resizable_parallel_runner_cxx.h>
#include <jxl/types.h>

#include "jxl.hpp"
#include "log.hpp"

unsigned char *DecodeJXL(const unsigned char *data, size_t data_size,
                         uint32_t *width, uint32_t *height) {
    JxlDecoderPtr dec = nullptr;
    JxlBasicInfo info;
    JxlPixelFormat format = {4, JXL_TYPE_UINT8, JXL_NATIVE_ENDIAN, 0};
    uint32_t w = 0, h = 0;
    size_t buffer_size = 0;
    unsigned char *buffer = nullptr;

    // Multi-threaded parallel runner.
    JxlResizableParallelRunnerPtr runner = JxlResizableParallelRunnerMake(nullptr);

    dec = JxlDecoderMake(nullptr);
    if (JxlDecoderSubscribeEvents(dec.get(),
                                  JXL_DEC_BASIC_INFO | JXL_DEC_FULL_IMAGE) != JXL_DEC_SUCCESS) {
        LogPrint(ERR, "JXL decoder: JxlDecoderSubscribeEvents failed");
        goto err;
    }

    if (JxlDecoderSetParallelRunner(dec.get(),
                                    JxlResizableParallelRunner, runner.get()) != JXL_DEC_SUCCESS) {
        LogPrint(ERR, "JXL decoder: JxlDecoderSetParallelRunner failed");
        goto err;
    }

    JxlDecoderSetInput(dec.get(), data, data_size);
    JxlDecoderCloseInput(dec.get());

    while (true) {
        JxlDecoderStatus status = JxlDecoderProcessInput(dec.get());

        switch (status) {
        case JXL_DEC_ERROR: {
            LogPrint(ERR, "JXL decoder: Decoder error");
            goto err;
        }
        case JXL_DEC_NEED_MORE_INPUT: {
            LogPrint(ERR, "JXL decoder: Error, already provided all input");
            goto err;
        }
        case JXL_DEC_BASIC_INFO: {
            if (JXL_DEC_SUCCESS != JxlDecoderGetBasicInfo(dec.get(), &info)) {
                LogPrint(ERR, "JXL decoder: JxlDecoderGetBasicInfo failed");
                goto err;
            }
            w = info.xsize;
            h = info.ysize;
            JxlResizableParallelRunnerSetThreads(runner.get(),
                                                 JxlResizableParallelRunnerSuggestThreads(w, h));
            break;
        }
        case JXL_DEC_NEED_IMAGE_OUT_BUFFER: {
            if (JxlDecoderImageOutBufferSize(dec.get(),
                                             &format, &buffer_size) != JXL_DEC_SUCCESS) {
                LogPrint(ERR, "JXL decoder: JxlDecoderImageOutBufferSize failed");
                goto err;
            }
            if (buffer_size != w * h * 4) {
                LogPrint(ERR, "JXL decoder: Invalid out buffer size %d %d",
                         buffer_size, w * h * 4);
                goto err;
            }
            buffer = (unsigned char *)malloc(buffer_size);
            if (JXL_DEC_SUCCESS != JxlDecoderSetImageOutBuffer(dec.get(), &format,
                                                               buffer, buffer_size)) {
                LogPrint(ERR, "JXL decoder: JxlDecoderSetImageOutBuffer failed");
                goto err;
            }
            break;
        }
        case JXL_DEC_FULL_IMAGE: {
            // Nothing to do. Do not yet return. If the image is an animation, more
            // full frames may be decoded. This example only keeps the last one.
            break;
        }
        case JXL_DEC_SUCCESS: {
            // All decoding successfully finished.
            // It's not required to call JxlDecoderReleaseInput(dec.get()) here since
            // the decoder will be destroyed.
            goto success;
        }
        default: {
            LogPrint(ERR, "JXL decoder: Unknown decoder status");
            goto err;
        }
        }
    }

success:
    *width = w;
    *height = h;
    return buffer;

err:
    free(buffer);
    return nullptr;
}

unsigned char *EncodeJXL(unsigned char *src_data, size_t src_data_size,
                         uint32_t src_width, uint32_t src_height, size_t *out_size) {
    LogPrint(ERR, "JXL encoder: JXL encoding is a TODO and this is a stub");

    *out_size = 0;
    return nullptr;
}

#else // #ifdef SSEDIT_HAVE_LIBJXL

#include "jxl.hpp"
#include "log.hpp"

unsigned char *DecodeJXL(const unsigned char *data, size_t data_size,
                         uint32_t *width, uint32_t *height) {
    LogPrint(ERR, "JXL decoder: ssedit was compiled without JPEG support, how did you get here?");

    *width = 0;
    *height = 0;
    return nullptr;
}

unsigned char *EncodeJXL(unsigned char *src_data, size_t src_data_size,
                         uint32_t src_width, uint32_t src_height, size_t *out_size) {
    LogPrint(ERR, "JXL encoder: ssedit was compiled without JPEG support, how did you get here?");

    return nullptr;
}

#endif // #ifdef SSEDIT_HAVE_LIBJXL

