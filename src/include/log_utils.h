#ifndef LOG_UTILS_H_
#define LOG_UTILS_H_
#include "common.h"

#define LOG_DEBUG 0
#define LOG_INFO 1
#define LOG_ERROR 2

void init_log_level(void);
void Log(int level, const char *filename, const char *func, int line, const char *format, ...);

#define PRINT_DEBUG(format, ...) do { \
    Log(LOG_DEBUG, __FILE__, __func__, __LINE__, format, ##__VA_ARGS__); \
} while(0)

#define PRINT_INFO(format, ...) do { \
    Log(LOG_INFO, __FILE__, __func__, __LINE__, format, ##__VA_ARGS__); \
} while(0)

#define PRINT_ERROR(format, ...) do { \
    Log(LOG_ERROR, __FILE__, __func__, __LINE__, format, ##__VA_ARGS__); \
} while(0)

#endif
