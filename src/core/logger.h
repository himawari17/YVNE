#ifndef YVNE_CORE_LOGGER_H
#define YVNE_CORE_LOGGER_H

typedef enum {
    YVNE_LOG_LEVEL_DEBUG,
    YVNE_LOG_LEVEL_INFO,
    YVNE_LOG_LEVEL_WARN,
    YVNE_LOG_LEVEL_ERROR,
} YVNE_LogLevel;

#if defined(__GNUC__) || defined(__clang__)
#define YVNE_PRINTF_LIKE(format_index, first_argument) \
    __attribute__((format(printf, format_index, first_argument)))
#else
#define YVNE_PRINTF_LIKE(format_index, first_argument)
#endif

void YVNE_LogWrite(YVNE_LogLevel level, const char *file, int line,
                   const char *format, ...)
    YVNE_PRINTF_LIKE(4, 5);

#undef YVNE_PRINTF_LIKE

#ifdef YVNE_DEBUG
#define YVNE_LOG_DEBUG(...) \
    YVNE_LogWrite(YVNE_LOG_LEVEL_DEBUG, __FILE_NAME__, __LINE__, __VA_ARGS__)
#define YVNE_LOG_INFO(...) \
    YVNE_LogWrite(YVNE_LOG_LEVEL_INFO, __FILE_NAME__, __LINE__, __VA_ARGS__)
#else
#define YVNE_LOG_DEBUG(...) ((void)0)
#define YVNE_LOG_INFO(...) ((void)0)
#endif

#define YVNE_LOG_WARN(...) \
    YVNE_LogWrite(YVNE_LOG_LEVEL_WARN, __FILE_NAME__, __LINE__, __VA_ARGS__)
#define YVNE_LOG_ERROR(...) \
    YVNE_LogWrite(YVNE_LOG_LEVEL_ERROR, __FILE_NAME__, __LINE__, __VA_ARGS__)

#endif
