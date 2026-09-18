#include "logger.h"

#include <SDL3/SDL_time.h>

#include <stdarg.h>
#include <stdio.h>

static void YVNE_LogTimestamp(char *buffer, size_t buffer_size){
    SDL_Time now;
    SDL_DateTime local;
    if (SDL_GetCurrentTime(&now) && SDL_TimeToDateTime(now, &local, true)){
        snprintf(buffer, buffer_size,
                 "%04d-%02d-%02d %02d:%02d:%02d.%03d",
                 local.year, local.month, local.day,
                 local.hour, local.minute, local.second,
                 local.nanosecond / 1'000'000);
        return;
    }
    snprintf(buffer, buffer_size, "unknown-time");
}

static const char *YVNE_LogLevelName(YVNE_LogLevel level){
    static const char *const names[] = {"DEBUG", "INFO", "WARN", "ERROR"};
    if (level < YVNE_LOG_LEVEL_DEBUG || level > YVNE_LOG_LEVEL_ERROR) {
        return "UNKNOWN";
    }
    return names[level];
}

static const char *YVNE_LogFileName(const char *path){
    const char *name = path;
    for (const char *cursor = path; *cursor != '\0'; ++cursor) {
        if (*cursor == '/' || *cursor == '\\') {
            name = cursor + 1;
        }
    }
    return name;
}

void YVNE_LogWrite(YVNE_LogLevel level, const char *file, int line,
                   const char *format, ...){
    char timestamp[24];
    va_list arguments;

    YVNE_LogTimestamp(timestamp, sizeof timestamp);
    fprintf(stderr, "[%s] [%s] %s:%d: ", timestamp,
            YVNE_LogLevelName(level), YVNE_LogFileName(file), line);
    va_start(arguments, format);
    vfprintf(stderr, format, arguments);
    va_end(arguments);
    fputc('\n', stderr);
}
