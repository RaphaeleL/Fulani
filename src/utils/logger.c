#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "../../include/utils.h"

static int debug_enabled = 0;

void set_debug_mode(int enabled) {
    debug_enabled = enabled;
}

void log_message(LogLevel level, const char* format, ...) {
    // Skip debug messages if debug mode is not enabled
    if (level == LOG_DEBUG && !debug_enabled) {
        return;
    }
    
    // Get current time
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    
    // Get level string
    const char* level_str;
    switch (level) {
        case LOG_INFO:
            level_str = "INFO";
            break;
        case LOG_WARNING:
            level_str = "WARNING";
            break;
        case LOG_ERROR:
            level_str = "ERROR";
            break;
        case LOG_DEBUG:
            level_str = "DEBUG";
            break;
        default:
            level_str = "UNKNOWN";
            break;
    }
    
    // Print log header
    fprintf(stderr, "[%s] [%s] ", time_str, level_str);
    
    // Print log message
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    
    fprintf(stderr, "\n");
} 