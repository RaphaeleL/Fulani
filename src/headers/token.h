#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    // Keywords
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_STRING,
    TOKEN_VOID,
    TOKEN_RETURN,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FOR,      // For loop keyword
    TOKEN_BOOL,     // New boolean type
    TOKEN_LIST,     // New list type
    TOKEN_DOUBLE,   // New double type
    TOKEN_LONG,     // New long type
    TOKEN_ADD,      // For list.add method
    TOKEN_REMOVE,   // For list.remove method
    TOKEN_LENGTH,   // For list.length property

    // Identifiers and literals
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_STRING_LITERAL,
    TOKEN_BOOL_LITERAL,    // New boolean literal

    // Operators
    TOKEN_PLUS,          // +
    TOKEN_MINUS,         // -
    TOKEN_MULTIPLY,      // *
    TOKEN_DIVIDE,        // /
    TOKEN_MODULO,        // %
    TOKEN_ASSIGN,        // =
    TOKEN_EQUALS,        // ==
    TOKEN_NOT_EQUALS,    // !=
    TOKEN_LESS,         // <
    TOKEN_GREATER,      // >
    TOKEN_LESS_EQUAL,   // <=
    TOKEN_GREATER_EQUAL, // >=
    TOKEN_BANG,         // !
    TOKEN_CONCAT,       // + for string concatenation

    // Delimiters
    TOKEN_LPAREN,       // (
    TOKEN_RPAREN,       // )
    TOKEN_LBRACE,       // {
    TOKEN_RBRACE,       // }
    TOKEN_COMMA,        // ,
    TOKEN_SEMICOLON,    // ;
    TOKEN_DOT,          // .
    TOKEN_LBRACKET,     // [ for list access
    TOKEN_RBRACKET,     // ] for list access

    // Special tokens
    TOKEN_EOF,
    TOKEN_ERROR
} TokenType;

typedef struct {
    TokenType type;
    char* lexeme;
    int line;
    int column;
} Token;

#endif // TOKEN_H 