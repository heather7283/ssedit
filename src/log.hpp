#pragma once

#include <stdio.h>

enum LogLevel {
    LOGLEVEL_SILENT,
    ERR,
    WARN,
    INFO,
    LOGLEVEL_MAX
};

void LogInit(LogLevel max_level, FILE *stream);
void LogPrint(LogLevel level, const char *fmt, ...);

