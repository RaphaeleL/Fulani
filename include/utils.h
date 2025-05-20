#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Logger
typedef enum {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_DEBUG
} LogLevel;

void log_message(LogLevel level, const char* format, ...);
void set_debug_mode(int enabled);

// String utilities
char* string_duplicate(const char* str);
char* string_concat(const char* str1, const char* str2);
char* string_substring(const char* str, int start, int length);
int string_to_int(const char* str);
double string_to_double(const char* str);

// File utilities
char* read_file(const char* filename);
int write_file(const char* filename, const char* content);

#endif /* UTILS_H */ 