#ifdef SSEDIT_HAVE_LIBJXL

#include <jxl/codestream_header.h>
#include <jxl/color_encoding.h>
#include <jxl/decode.h>
#include <jxl/decode_cxx.h>
#include <jxl/encode.h>
#include <jxl/encode_cxx.h>
#include <jxl/thread_parallel_runner.h>
#include <jxl/thread_parallel_runner_cxx.h>
#include <jxl/resizable_parallel_runner.h>
#include <jxl/resizable_parallel_runner_cxx.h>
#include <jxl/types.h>

#include "jxl.hpp"
#include "log.hpp"

unsigned char *DecodeJXL(const unsigned char *data, size_t data_size,
                         uint32_t *width, uint32_t *height) {
    JxlDecoderPtr dec = nullptr;
    JxlBasicInfo info;
    JxlPixelFormat format = {3, JXL_TYPE_UINT8, JXL_NATIVE_ENDIAN, 0};
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
            if (buffer_size != w * h * 3) {
                LogPrint(ERR, "JXL decoder: Invalid out buffer size %d %d",
                         buffer_size, w * h * 3);
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
    JxlEncoderPtr enc = nullptr;
    JxlThreadParallelRunnerPtr runner = nullptr;
    JxlPixelFormat pixel_format = {3, JXL_TYPE_UINT8, JXL_NATIVE_ENDIAN, 0};
    JxlBasicInfo basic_info;
    JxlColorEncoding color_encoding = {};
    JxlEncoderFrameSettings *frame_settings;
    unsigned char *out_buf = nullptr;
    size_t out_buf_size = 0;
    size_t avail_out;
    JxlEncoderStatus process_result;
    unsigned char *next_out;

    enc = JxlEncoderMake(nullptr);
    runner = JxlThreadParallelRunnerMake(nullptr,
                                         JxlThreadParallelRunnerDefaultNumWorkerThreads());
    if (JxlEncoderSetParallelRunner(enc.get(),
                                    JxlThreadParallelRunner, runner.get()) != JXL_ENC_SUCCESS) {
        LogPrint(ERR, "JXL encoder: JxlEncoderSetParallelRunner failed");
        goto err;
    }

    JxlEncoderInitBasicInfo(&basic_info);
    basic_info.xsize = src_width;
    basic_info.ysize = src_height;
    basic_info.bits_per_sample = 8;
    basic_info.exponent_bits_per_sample = 0;
    basic_info.alpha_bits = 0;
    basic_info.num_color_channels = 3;
    basic_info.num_extra_channels = 0;
    basic_info.uses_original_profile = JXL_TRUE;
    if (JxlEncoderSetBasicInfo(enc.get(), &basic_info) != JXL_ENC_SUCCESS) {
        LogPrint(ERR, "JXL encoder: JxlEncoderSetBasicInfo failed");
        goto err;
    }

    JxlColorEncodingSetToSRGB(&color_encoding, false);
    if (JxlEncoderSetColorEncoding(enc.get(), &color_encoding) != JXL_ENC_SUCCESS) {
        LogPrint(ERR, "JXL encoder: JxlEncoderSetColorEncoding failed");
        goto err;
    }

    frame_settings = JxlEncoderFrameSettingsCreate(enc.get(), nullptr);
    if (frame_settings == nullptr) {
        LogPrint(ERR, "JXL encoder: JxlEncoderFrameSettingsCreate failed");
        goto err;
    }

    if (JxlEncoderAddImageFrame(frame_settings, &pixel_format,
                                src_data, src_data_size) != JXL_ENC_SUCCESS) {
        LogPrint(ERR, "JXL encoder: JxlEncoderAddImageFrame failed");
        goto err;
    }
    JxlEncoderCloseInput(enc.get());

    out_buf = (unsigned char *)malloc(32768);
    out_buf_size = 32768;

    next_out = out_buf;
    avail_out = out_buf_size;
    process_result = JXL_ENC_NEED_MORE_OUTPUT;
    while (process_result == JXL_ENC_NEED_MORE_OUTPUT) {
        process_result = JxlEncoderProcessOutput(enc.get(), &next_out, &avail_out);
        if (process_result == JXL_ENC_NEED_MORE_OUTPUT) {
            size_t offset = next_out - out_buf;
            out_buf = (unsigned char *)realloc(out_buf, out_buf_size *= 2);

            next_out = out_buf + offset;
            avail_out = out_buf_size - offset;
        }
    }
    if (process_result != JXL_ENC_SUCCESS) {
        LogPrint(ERR, "JXL encoder: JxlEncoderProcessOutput failed");
        goto err;
    }
    out_buf_size = next_out - out_buf;
    out_buf = (unsigned char *)realloc(out_buf, out_buf_size);

    *out_size = out_buf_size;
    return out_buf;

err:
    free(out_buf);
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

