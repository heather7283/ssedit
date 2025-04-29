#pragma once

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

