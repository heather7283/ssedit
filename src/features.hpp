#pragma once

#ifndef SSEDIT_LIBSPNG_VERSION
#define SSEDIT_LIBSPNG_VERSION "none"
#endif

#ifndef SSEDIT_LIBTURBOJPEG_VERSION
#define SSEDIT_LIBTURBOJPEG_VERSION "none"
#endif

#ifndef SSEDIT_LIBJXL_VERSION
#define SSEDIT_LIBJXL_VERSION "none"
#endif

inline bool HasFeaturePNG(void) {
#ifdef SSEDIT_HAVE_LIBSPNG
    return true;
#else
    return false;
#endif
}

inline bool HasFeatureJPEG(void) {
#ifdef SSEDIT_HAVE_LIBTURBOJPEG
    return true;
#else
    return false;
#endif
}

inline bool HasFeatureJXL(void) {
#ifdef SSEDIT_HAVE_LIBJXL
    return true;
#else
    return false;
#endif
}

