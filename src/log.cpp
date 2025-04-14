#include <unistd.h>
#include <stdarg.h>

#include "log.hpp"

#define ANSI_ERROR "\033[1;31m"
#define ANSI_WARN  "\033[1;33m"
#define ANSI_INFO  "\033[34m"
#define ANSI_DEBUG "\033[90m"
#define ANSI_RESET "\033[0m"

static struct {
    FILE *stream;
    bool colors;
    LogLevel max_level;
} config = {
    .stream = NULL,
    .colors = false,
    .max_level = INFO,
};

void LogInit(LogLevel max_level, FILE *stream) {
    config.max_level = max_level;
    config.stream = stream;
    config.colors = isatty(fileno(stream));
}

void LogPrint(LogLevel level, const char *fmt, ...) {
    if (level > config.max_level) {
        return;
    }

    const char *level_str;
    const char *color_str;
    switch (level) {
    case INFO:
        level_str = "[INFO] ";
        color_str = ANSI_INFO;
        break;
    case WARN:
        level_str = "[WARN] ";
        color_str = ANSI_WARN;
        break;
    case ERR:
        level_str = "[ERROR] ";
        color_str = ANSI_ERROR;
        break;
    default:
        level_str = "[?????] ";
        color_str = ANSI_ERROR;
    }

    if (config.colors) {
        fputs(color_str, config.stream);
    }
    fputs(level_str, config.stream);

    va_list args;
    va_start(args, fmt);
    vfprintf(config.stream, fmt, args);
    va_end(args);

    if (config.colors) {
        fputs(ANSI_RESET, config.stream);
    }
    fputc('\n', config.stream);
    fflush(config.stream);
}

