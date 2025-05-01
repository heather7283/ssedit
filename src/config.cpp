#include <cerrno>
#include <cstdlib>
#include <climits>

#include "config.hpp"
#include "log.hpp"

struct Config config;

static const char *GetDefaultConfigPath(void) {
    static char path[PATH_MAX];

    const char *config_home = getenv("XDG_CONFIG_HOME");
    const char *home = getenv("HOME");
    if (config_home != nullptr) {
        snprintf(path, sizeof(path), "%s/ssedit/config.ini", config_home);
    } else if (home != nullptr) {
        snprintf(path, sizeof(path), "%s/.config/ssedit/config.ini", home);
    } else {
        LogPrint(ERR, "Config: both XDG_CONFIG_HOME and HOME are unset!");
        return nullptr;
    }

    return path;
}

static bool HexStringToVec(const char *hex, ImVec4 *vec) {
    long color;
    char *endptr;
    bool has_alpha = true;
    int hex_len = strlen(hex);

    if ((hex_len == 7 || hex_len == 9) && hex[0] == '#') {
        hex_len--; hex++;
    }

    if (hex_len == 6) {
        has_alpha = false;
    } else if (hex_len != 8) {
        goto err;
    }

    errno = 0;
    color = strtol(hex, &endptr, 16);
    if (errno != 0 || *endptr != '\0') {
        goto err;
    }
    if (!has_alpha) {
        color <<= 8;
        color |= 0x000000FF;
    }

    vec->x = ((color & 0xFF000000) >> 24) / 255.0f;
    vec->y = ((color & 0x00FF0000) >> 16) / 255.0f;
    vec->z = ((color & 0x0000FF00) >>  8) / 255.0f;
    vec->w = ((color & 0x000000FF) >>  0) / 255.0f;

    return true;

err:
    LogPrint(ERR, "Config: could not convert %s to color", hex);
    return false;
}

static bool StringToFloat(const char *str, float *f) {
    float local_f;
    char *endptr;

    errno = 0;
    local_f = strtof(str, &endptr);
    if (errno != 0 || *endptr != '\0') {
        goto err;
    }

    *f = local_f;
    return true;

err:
    LogPrint(ERR, "Config: could not convert %s to float", str);
    return false;
}

static int ConfigHandler(void *data, const char *section, const char *name, const char *value) {
    #define MATCH(s, n) ((strcmp(section, s) == 0) && (strcmp(name, n) == 0))

    ImGuiStyle *style = (ImGuiStyle *)data;

    if (MATCH("Colors", "Text")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_Text]);
    } else if (MATCH("Colors", "TextDisabled")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_TextDisabled]);
    } else if (MATCH("Colors", "WindowBg")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_WindowBg]);
    } else if (MATCH("Colors", "ChildBg")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_ChildBg]);
    } else if (MATCH("Colors", "PopupBg")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_PopupBg]);
    } else if (MATCH("Colors", "Border")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_Border]);
    } else if (MATCH("Colors", "BorderShadow")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_BorderShadow]);
    } else if (MATCH("Colors", "FrameBg")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_FrameBg]);
    } else if (MATCH("Colors", "FrameBgHovered")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_FrameBgHovered]);
    } else if (MATCH("Colors", "FrameBgActive")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_FrameBgActive]);
    } else if (MATCH("Colors", "TitleBg")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_TitleBg]);
    } else if (MATCH("Colors", "TitleBgActive")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_TitleBgActive]);
    } else if (MATCH("Colors", "TitleBgCollapsed")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_TitleBgCollapsed]);
    } else if (MATCH("Colors", "MenuBarBg")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_MenuBarBg]);
    } else if (MATCH("Colors", "ScrollbarBg")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_ScrollbarBg]);
    } else if (MATCH("Colors", "ScrollbarGrab")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_ScrollbarGrab]);
    } else if (MATCH("Colors", "ScrollbarGrabHovered")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_ScrollbarGrabHovered]);
    } else if (MATCH("Colors", "ScrollbarGrabActive")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_ScrollbarGrabActive]);
    } else if (MATCH("Colors", "CheckMark")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_CheckMark]);
    } else if (MATCH("Colors", "SliderGrab")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_SliderGrab]);
    } else if (MATCH("Colors", "SliderGrabActive")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_SliderGrabActive]);
    } else if (MATCH("Colors", "Button")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_Button]);
    } else if (MATCH("Colors", "ButtonHovered")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_ButtonHovered]);
    } else if (MATCH("Colors", "ButtonActive")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_ButtonActive]);
    } else if (MATCH("Colors", "Header")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_Header]);
    } else if (MATCH("Colors", "HeaderHovered")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_HeaderHovered]);
    } else if (MATCH("Colors", "HeaderActive")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_HeaderActive]);
    } else if (MATCH("Colors", "Separator")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_Separator]);
    } else if (MATCH("Colors", "SeparatorHovered")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_SeparatorHovered]);
    } else if (MATCH("Colors", "SeparatorActive")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_SeparatorActive]);
    } else if (MATCH("Colors", "ResizeGrip")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_ResizeGrip]);
    } else if (MATCH("Colors", "ResizeGripHovered")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_ResizeGripHovered]);
    } else if (MATCH("Colors", "ResizeGripActive")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_ResizeGripActive]);
    } else if (MATCH("Colors", "Tab")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_Tab]);
    } else if (MATCH("Colors", "TabHovered")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_TabHovered]);
    } else if (MATCH("Colors", "TabSelected")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_TabSelected]);
    } else if (MATCH("Colors", "TabSelectedOverline")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_TabSelectedOverline]);
    } else if (MATCH("Colors", "TabDimmed")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_TabDimmed]);
    } else if (MATCH("Colors", "TabDimmedSelected")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_TabDimmedSelected]);
    } else if (MATCH("Colors", "TabDimmedSelectedOverline")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_TabDimmedSelectedOverline]);
    } else if (MATCH("Colors", "PlotLines")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_PlotLines]);
    } else if (MATCH("Colors", "PlotLinesHovered")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_PlotLinesHovered]);
    } else if (MATCH("Colors", "PlotHistogram")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_PlotHistogram]);
    } else if (MATCH("Colors", "PlotHistogramHovered")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_PlotHistogramHovered]);
    } else if (MATCH("Colors", "TableHeaderBg")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_TableHeaderBg]);
    } else if (MATCH("Colors", "TableBorderStrong")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_TableBorderStrong]);
    } else if (MATCH("Colors", "TableBorderLight")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_TableBorderLight]);
    } else if (MATCH("Colors", "TableRowBg")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_TableRowBg]);
    } else if (MATCH("Colors", "TableRowBgAlt")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_TableRowBgAlt]);
    } else if (MATCH("Colors", "TextLink")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_TextLink]);
    } else if (MATCH("Colors", "TextSelectedBg")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_TextSelectedBg]);
    } else if (MATCH("Colors", "DragDropTarget")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_DragDropTarget]);
    } else if (MATCH("Colors", "NavCursor")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_NavCursor]);
    } else if (MATCH("Colors", "NavWindowingHighlight")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_NavWindowingHighlight]);
    } else if (MATCH("Colors", "NavWindowingDimBg")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_NavWindowingDimBg]);
    } else if (MATCH("Colors", "ModalWindowDimBg")) {
        HexStringToVec(value, &style->Colors[ImGuiCol_ModalWindowDimBg]);
    } else if (MATCH("Main", "FontSize")) {
        StringToFloat(value, &config.font_size);
    } else if (MATCH("Main", "FontFile")) {
        config.font_path = strdup(value);
    } else {
        LogPrint(WARN, "Config: unknown option %s in section %s", name, section);
    }

    return 1;
}

bool LoadConfig(const char *config_file_path, ImGuiStyle *style) {
    if (config_file_path == nullptr) {
        config_file_path = GetDefaultConfigPath();
    }
    if (config_file_path == nullptr) {
        LogPrint(ERR, "Config: could not determine config file path!");
        return false;
    }

    LogPrint(INFO, "Config: reading %s", config_file_path);
    if (ini_parse(config_file_path, ConfigHandler, style) < 0) {
        LogPrint(ERR, "Config: failed to load config");
        return false;
    }

    return true;
}

