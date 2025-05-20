#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../include/utils.h"

void test_string_utils() {
    printf("Testing string_utils...\n");
    
    // Test string_duplicate
    char* original = "Hello";
    char* copy = string_duplicate(original);
    assert(copy != NULL);
    assert(strcmp(copy, original) == 0);
    free(copy);
    
    // Test string_concat
    char* str1 = "Hello, ";
    char* str2 = "World!";
    char* result = string_concat(str1, str2);
    assert(result != NULL);
    assert(strcmp(result, "Hello, World!") == 0);
    free(result);
    
    printf("String utils tests passed!\n");
}

int main() {
    printf("Running tests...\n");
    
    test_string_utils();
    
    printf("All tests passed!\n");
    return 0;
} 