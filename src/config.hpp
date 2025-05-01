#pragma once

#include <ini.h>
#include <imgui/imgui.h>

#define RGBA_TO_IMVEC4(r, g, b, a) ImVec4((r) / 255.f, (g) / 255.f, (b) / 255.f, (a) / 255.f)

struct Config {
    struct {
        ImVec4 bg = RGBA_TO_IMVEC4(0x23, 0x2A, 0x2E, 0xFF);
        ImVec4 popup_bg = RGBA_TO_IMVEC4(0x3D, 0x48, 0x4D, 0xFF);
        ImVec4 accent = RGBA_TO_IMVEC4(0xA7, 0xC0, 0x80, 0xFF);
        ImVec4 text = RGBA_TO_IMVEC4(0xD3, 0xC6, 0xAA, 0xFF);
    } colors;
    float font_size = 18.0f;
    const char *font_path = nullptr;
};

extern struct Config config;

bool LoadConfig(const char *config_file_path);

