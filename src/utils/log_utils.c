#include "log_utils.h"

static int current_log_level = LOG_INFO;

void init_log_level(void) {
#if defined(LOG_LEVEL_DEBUG)
    current_log_level = LOG_DEBUG;
#elif defined(LOG_LEVLE_INFO)
    current_log_level = LOG_INFO;
#else
    current_log_level = LOG_ERROR;
#endif
}

void Log(int level, const char *filename, const char *func, int line, const char *format, ...) {
    if (level < current_log_level) {
        return;
    }

    va_list args;
    va_start(args, format);
    switch (level) {
        case LOG_DEBUG:
            fprintf(stderr, "[%s:%s:%d]: ", filename, func, line);
            break;
        case LOG_INFO:
            fprintf(stderr, "[%s:%s:%d]: ", filename, func, line);
            break;
        case LOG_ERROR:
            fprintf(stderr, "[%s:%s:%d]: ", filename, func, line);
            break;
        default:
            fprintf(stderr, "UNKNOWN [%s:%s:%d]: ", filename, func, line);
            break;
    }
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}
