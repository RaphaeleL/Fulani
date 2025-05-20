#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/utils.h"

char* string_duplicate(const char* str) {
    if (str == NULL) {
        return NULL;
    }
    
    size_t len = strlen(str) + 1;
    char* result = (char*)malloc(len);
    
    if (result != NULL) {
        memcpy(result, str, len);
    }
    
    return result;
}

char* string_concat(const char* str1, const char* str2) {
    if (str1 == NULL || str2 == NULL) {
        return NULL;
    }
    
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);
    char* result = (char*)malloc(len1 + len2 + 1);
    
    if (result != NULL) {
        memcpy(result, str1, len1);
        memcpy(result + len1, str2, len2 + 1);
    }
    
    return result;
}

char* string_substring(const char* str, int start, int length) {
    if (str == NULL || start < 0 || length <= 0) {
        return NULL;
    }
    
    size_t str_len = strlen(str);
    if ((size_t)start >= str_len) {
        return NULL;
    }
    
    if ((size_t)(start + length) > str_len) {
        length = str_len - start;
    }
    
    char* result = (char*)malloc(length + 1);
    
    if (result != NULL) {
        memcpy(result, str + start, length);
        result[length] = '\0';
    }
    
    return result;
}

int string_to_int(const char* str) {
    if (str == NULL) {
        return 0;
    }
    
    return atoi(str);
}

double string_to_double(const char* str) {
    if (str == NULL) {
        return 0.0;
    }
    
    return atof(str);
}

char* read_file(const char* filename) {
    if (filename == NULL) {
        return NULL;
    }
    
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        return NULL;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Allocate buffer
    char* buffer = (char*)malloc(file_size + 1);
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }
    
    // Read file content
    size_t read_size = fread(buffer, 1, file_size, file);
    buffer[read_size] = '\0';
    
    fclose(file);
    return buffer;
}

int write_file(const char* filename, const char* content) {
    if (filename == NULL || content == NULL) {
        return 0;
    }
    
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        return 0;
    }
    
    size_t content_len = strlen(content);
    size_t write_size = fwrite(content, 1, content_len, file);
    
    fclose(file);
    return (write_size == content_len);
} 