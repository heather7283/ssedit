#include "icons.hpp"

extern char _binary_icons_ttf_start;
void *icons_ttf_start = &_binary_icons_ttf_start;
extern char _binary_icons_ttf_end;
void *icons_ttf_end = &_binary_icons_ttf_end;

size_t icons_ttf_size = (char *)icons_ttf_end - (char *)icons_ttf_start;

