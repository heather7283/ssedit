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
    int hex_len = strlen(hex);

    if ((hex_len == 7 || hex_len == 9) && hex[0] == '#') {
        hex_len--; hex++;
    } else if (!(hex_len == 6 || hex_len == 8)) {
        goto err;
    }

    errno = 0;
    color = strtol(hex, &endptr, 16);
    if (errno != 0 || *endptr != '\0') {
        goto err;
    }

    vec->x = color | 0xFF000000;
    vec->y = color | 0x00FF0000;
    vec->z = color | 0x0000FF00;
    vec->w = color | 0x000000FF;

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

    if (MATCH("colors", "background")) {
        HexStringToVec(value, &config.bg_color);
    } else if (MATCH("colors", "accent")) {
        HexStringToVec(value, &config.accent_color);
    } else if (MATCH("colors", "text")) {
        HexStringToVec(value, &config.text_color);
    } else if (MATCH("main", "font-size")) {
        StringToFloat(value, &config.font_size);
    } else if (MATCH("main", "font-file")) {
        config.font_path = strdup(value);
    } else {
        LogPrint(WARN, "Config: unknown option %s in section %s", name, section);
    }

    return 1;
}

bool LoadConfig(const char *config_file_path) {
    if (config_file_path == nullptr) {
        config_file_path = GetDefaultConfigPath();
    }
    if (config_file_path == nullptr) {
        LogPrint(ERR, "Config: could not determine config file path!");
        return false;
    }

    LogPrint(INFO, "Config: reading %s", config_file_path);
    if (ini_parse(config_file_path, ConfigHandler, nullptr) < 0) {
        LogPrint(ERR, "Config: failed to load config");
        return false;
    }

    return true;
}

