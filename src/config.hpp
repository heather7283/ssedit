#pragma once

#include <ini.h>
#include <imgui/imgui.h>

#define RGBA_TO_IMVEC4(r, g, b, a) ImVec4((r) / 255.f, (g) / 255.f, (b) / 255.f, (a) / 255.f)

struct Config {
    ImVec4 bg_color = RGBA_TO_IMVEC4(0x23, 0x2A, 0x2E, 0xFF);
    ImVec4 accent_color = RGBA_TO_IMVEC4(0xA7, 0xC0, 0x80, 0xFF);
    ImVec4 text_color = RGBA_TO_IMVEC4(0x23, 0x2A, 0x2E, 0xFF);
    float font_size = 20.0f;
    const char *font_path = "/usr/share/fonts/nerdfonts/JetBrainsMonoNLNerdFont-Medium.ttf";
};

extern struct Config config;

bool LoadConfig(const char *config_file_path);

