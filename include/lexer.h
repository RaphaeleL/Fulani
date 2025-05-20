#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Token types
typedef enum {
    TOKEN_EOF = 0,
    TOKEN_IDENTIFIER,
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_STRING,
    
    // Keywords
    TOKEN_INT_KW,
    TOKEN_FLOAT_KW,
    TOKEN_CHAR_KW,
    TOKEN_DOUBLE_KW,
    TOKEN_LONG_KW,
    TOKEN_STRING_KW,
    TOKEN_LIST_KW,
    TOKEN_STRUCT_KW,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_FOR,
    TOKEN_WHILE,
    TOKEN_VOID,
    TOKEN_INCLUDE,
    TOKEN_RETURN,
    
    // Operators and symbols
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_ASSIGN,
    TOKEN_EQ,
    TOKEN_NEQ,
    TOKEN_LT,
    TOKEN_GT,
    TOKEN_LE,
    TOKEN_GE,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_SEMICOLON,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_COLON,
    TOKEN_HASH
} TokenType;

// Token structure
typedef struct {
    TokenType type;
    char* value;
    int line;
    int column;
} Token;

// Lexer structure
typedef struct {
    char* source;
    size_t source_len;
    size_t pos;
    size_t line;
    size_t column;
    Token current_token;
} Lexer;

// Function prototypes
Lexer* lexer_init(char* source);
void lexer_free(Lexer* lexer);
Token lexer_get_next_token(Lexer* lexer);
const char* token_type_to_string(TokenType type);

#endif /* LEXER_H */ 