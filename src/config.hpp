#pragma once

#include <ini.h>
#include <imgui/imgui.h>

#define RGBA_TO_IMVEC4(r, g, b, a) ImVec4((r) / 255.f, (g) / 255.f, (b) / 255.f, (a) / 255.f)

struct Config {
    float font_size = 18.0f;
    const char *font_path = nullptr;
    // absolute value if > 0, relative to image size if < 0
    float initial_thickness = 0.10f;
};

extern struct Config config;

bool LoadConfig(const char *config_file_path, ImGuiStyle *style);

