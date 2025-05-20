#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/lexer.h"
#include "../include/utils.h"

void print_token(Token token) {
    printf("Token: %-15s | Value: %-15s | Line: %d | Column: %d\n",
           token_type_to_string(token.type),
           token.value ? token.value : "NULL",
           token.line,
           token.column);
}

void test_lexer_basic() {
    printf("Testing basic lexer functionality...\n");
    
    char* source = "int x = 5;";
    Lexer* lexer = lexer_init(source);
    
    // Expected tokens: INT_KW, IDENTIFIER, ASSIGN, INT, SEMICOLON, EOF
    Token token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_INT_KW);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_IDENTIFIER);
    assert(strcmp(token.value, "x") == 0);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_ASSIGN);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_INT);
    assert(strcmp(token.value, "5") == 0);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_SEMICOLON);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_EOF);
    
    lexer_free(lexer);
    printf("Basic lexer test passed!\n");
}

void test_lexer_operators() {
    printf("Testing lexer operator recognition...\n");
    
    char* source = "+ - * / = == != < > <= >=";
    Lexer* lexer = lexer_init(source);
    
    Token token;
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_PLUS);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_MINUS);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_MULTIPLY);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_DIVIDE);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_ASSIGN);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_EQ);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_NEQ);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_LT);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_GT);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_LE);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_GE);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_EOF);
    
    lexer_free(lexer);
    printf("Operator test passed!\n");
}

void test_lexer_numbers() {
    printf("Testing lexer number recognition...\n");
    
    char* source = "123 45.67 3.14159 1e10 2.5e-3";
    Lexer* lexer = lexer_init(source);
    
    Token token;
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_INT);
    assert(strcmp(token.value, "123") == 0);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_FLOAT);
    assert(strcmp(token.value, "45.67") == 0);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_FLOAT);
    assert(strcmp(token.value, "3.14159") == 0);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_FLOAT);
    assert(strcmp(token.value, "1e10") == 0);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_FLOAT);
    assert(strcmp(token.value, "2.5e-3") == 0);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_EOF);
    
    lexer_free(lexer);
    printf("Number test passed!\n");
}

void test_lexer_strings() {
    printf("Testing lexer string recognition...\n");
    
    char* source = "\"Hello, world!\" \"String with \\\"escaped quotes\\\"\"";
    Lexer* lexer = lexer_init(source);
    
    Token token;
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_STRING);
    assert(strcmp(token.value, "Hello, world!") == 0);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_STRING);
    assert(strcmp(token.value, "String with \\\"escaped quotes\\\"") == 0);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_EOF);
    
    lexer_free(lexer);
    printf("String test passed!\n");
}

void test_lexer_comments() {
    printf("Testing lexer comment handling...\n");
    
    char* source = "int x = 5; // This is a comment\n"
                  "/* This is a\n"
                  "   multi-line comment */\n"
                  "float y = 10.5;";
    Lexer* lexer = lexer_init(source);
    
    // Expected tokens: INT_KW, IDENTIFIER, ASSIGN, INT, SEMICOLON, FLOAT_KW, IDENTIFIER, ASSIGN, FLOAT, SEMICOLON, EOF
    Token token;
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_INT_KW);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_IDENTIFIER);
    assert(strcmp(token.value, "x") == 0);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_ASSIGN);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_INT);
    assert(strcmp(token.value, "5") == 0);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_SEMICOLON);
    free(token.value);
    
    // Comments should be skipped
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_FLOAT_KW);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_IDENTIFIER);
    assert(strcmp(token.value, "y") == 0);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_ASSIGN);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_FLOAT);
    assert(strcmp(token.value, "10.5") == 0);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_SEMICOLON);
    free(token.value);
    
    token = lexer_get_next_token(lexer);
    assert(token.type == TOKEN_EOF);
    
    lexer_free(lexer);
    printf("Comment test passed!\n");
}

void test_lexer_complete_program() {
    printf("Testing lexer with a complete program...\n");
    
    char* source = 
        "#include <io>\n"
        "\n"
        "// Simple program that calculates factorial\n"
        "int factorial(int n) {\n"
        "    if (n <= 1) {\n"
        "        return 1;\n"
        "    }\n"
        "    return n * factorial(n - 1);\n"
        "}\n"
        "\n"
        "int main() {\n"
        "    int num = 5;\n"
        "    int result = factorial(num);\n"
        "    print(\"Factorial of \" + num + \" is \" + result);\n"
        "    return 0;\n"
        "}\n";
    
    Lexer* lexer = lexer_init(source);
    
    // Tokenize the complete program
    Token token;
    int token_count = 0;
    
    while (1) {
        token = lexer_get_next_token(lexer);
        token_count++;
        
        // print_token(token); // Uncomment for debugging
        
        if (token.type == TOKEN_EOF) {
            break;
        }
        
        free(token.value);
    }
    
    printf("Successfully tokenized program with %d tokens\n", token_count);
    lexer_free(lexer);
}

int main() {
    printf("Running lexer tests...\n");
    
    test_lexer_basic();
    test_lexer_operators();
    test_lexer_numbers();
    test_lexer_strings();
    test_lexer_comments();
    test_lexer_complete_program();
    
    printf("All lexer tests passed!\n");
    return 0;
}