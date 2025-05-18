#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "headers/lexer.h"

static Token make_token(Lexer* lexer, TokenType type) {
    Token token;
    token.type = type;
    token.line = lexer->line;
    token.column = lexer->column;
    
    int length = lexer->current - lexer->start;
    token.lexeme = (char*)malloc(length + 1);
    strncpy(token.lexeme, &lexer->source[lexer->start], length);
    token.lexeme[length] = '\0';
    
    return token;
}

static Token error_token(Lexer* lexer, const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.line = lexer->line;
    token.column = lexer->column;
    token.lexeme = strdup(message);
    return token;
}

static char peek(Lexer* lexer) {
    return lexer->source[lexer->current];
}

static char peek_next(Lexer* lexer) {
    if (lexer->source[lexer->current] == '\0') return '\0';
    return lexer->source[lexer->current + 1];
}

static bool is_at_end(Lexer* lexer) {
    return peek(lexer) == '\0';
}

static char advance(Lexer* lexer) {
    lexer->current++;
    lexer->column++;
    return lexer->source[lexer->current - 1];
}

static bool match(Lexer* lexer, char expected) {
    if (is_at_end(lexer)) return false;
    if (lexer->source[lexer->current] != expected) return false;
    
    lexer->current++;
    lexer->column++;
    return true;
}

static void skip_whitespace(Lexer* lexer) {
    for (;;) {
        char c = peek(lexer);
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance(lexer);
                break;
            case '\n':
                lexer->line++;
                lexer->column = 0;
                advance(lexer);
                break;
            case '/':
                if (peek_next(lexer) == '/') {
                    // Single-line comment, skip until end of line
                    advance(lexer); // Skip the first '/'
                    advance(lexer); // Skip the second '/'
                    
                    while (peek(lexer) != '\n' && !is_at_end(lexer)) {
                        advance(lexer);
                    }
                } else if (peek_next(lexer) == '*') {
                    // Multi-line comment, skip until */
                    advance(lexer); // Skip the '/'
                    advance(lexer); // Skip the '*'
                    
                    int nesting = 1; // Track nesting level
                    
                    while (!is_at_end(lexer) && nesting > 0) {
                        if (peek(lexer) == '/' && peek_next(lexer) == '*') {
                            // Found a nested comment start /*
                            advance(lexer); // Skip '/'
                            advance(lexer); // Skip '*'
                            nesting++;
                        } else if (peek(lexer) == '*' && peek_next(lexer) == '/') {
                            // Found a comment end */
                            advance(lexer); // Skip '*'
                            advance(lexer); // Skip '/'
                            nesting--;
                        } else {
                            // Regular character inside comment
                            if (peek(lexer) == '\n') {
                                lexer->line++;
                                lexer->column = 0;
                            }
                            advance(lexer);
                        }
                    }
                    
                    if (nesting > 0) {
                        fprintf(stderr, "Warning: Unclosed comment at line %d\n", lexer->line);
                    }
                } else {
                    // Not a comment, just a division operator
                    return;
                }
                break;
            default:
                return;
        }
    }
}

static Token string(Lexer* lexer) {
    // Skip the opening quote
    advance(lexer);
    
    while (!is_at_end(lexer)) {
        if (peek(lexer) == '\\' && peek_next(lexer) == '"') {
            // Escaped quote, skip both backslash and quote
            advance(lexer); // Skip the backslash
            advance(lexer); // Skip the quote
        } else if (peek(lexer) == '"') {
            break; // End of string
        } else {
            if (peek(lexer) == '\n') {
                lexer->line++;
                lexer->column = 0;
            }
            advance(lexer);
        }
    }
    
    if (is_at_end(lexer)) {
        return error_token(lexer, "Unterminated string.");
    }
    
    // The closing quote.
    advance(lexer);
    
    // Create token with the string content (excluding quotes)
    Token token;
    token.type = TOKEN_STRING_LITERAL;
    token.line = lexer->line;
    token.column = lexer->column;
    
    // Allocate space for the string content
    int length = lexer->current - lexer->start - 2;  // -2 for the quotes
    token.lexeme = (char*)malloc(length + 1);
    
    // Copy the string content, handling escaped characters
    int j = 0;
    for (int i = lexer->start + 1; i < lexer->current - 1; i++) {
        if (lexer->source[i] == '\\' && i + 1 < lexer->current - 1) {
            if (lexer->source[i + 1] == '"') {
                token.lexeme[j++] = '"';
                i++; // Skip the next character (")
            } else {
                token.lexeme[j++] = lexer->source[i];
            }
        } else {
            token.lexeme[j++] = lexer->source[i];
        }
    }
    token.lexeme[j] = '\0';
    
    return token;
}

static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

static bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
            c == '_';
}

static Token number(Lexer* lexer) {
    while (is_digit(peek(lexer))) advance(lexer);
    
    // Look for a fractional part.
    if (peek(lexer) == '.' && is_digit(peek_next(lexer))) {
        // Consume the "."
        advance(lexer);
        
        while (is_digit(peek(lexer))) advance(lexer);
        return make_token(lexer, TOKEN_FLOAT_LITERAL);
    }
    
    return make_token(lexer, TOKEN_INTEGER_LITERAL);
}

static TokenType identifier_type(Lexer* lexer) {
    switch (lexer->source[lexer->start]) {
        case 'a': 
            if (lexer->current - lexer->start == 3 &&
                strncmp(lexer->source + lexer->start, "add", 3) == 0)
                return TOKEN_ADD;
            break;
        case 'b': {
            if (lexer->current - lexer->start > 1) {
                switch (lexer->source[lexer->start + 1]) {
                    case 'o':
                        if (lexer->current - lexer->start == 4 &&
                            strncmp(lexer->source + lexer->start + 2, "ol", 2) == 0) return TOKEN_BOOL;
                        break;
                }
            }
            break;
        }
        case 'd':
            if (lexer->current - lexer->start == 6 &&
                strncmp(lexer->source + lexer->start + 1, "ouble", 5) == 0)
                return TOKEN_DOUBLE;
            break;
        case 'e':
            if (lexer->current - lexer->start == 4 &&
                strncmp(lexer->source + lexer->start + 1, "lse", 3) == 0)
                return TOKEN_ELSE;
            break;
        case 'f':
            if (lexer->current - lexer->start == 5 &&
                strncmp(lexer->source + lexer->start + 1, "loat", 4) == 0)
                return TOKEN_FLOAT;
            else if (lexer->current - lexer->start == 3 &&
                strncmp(lexer->source + lexer->start, "for", 3) == 0)
                return TOKEN_FOR;
            break;
        case 'i': {
            if (lexer->current - lexer->start > 1) {
                switch (lexer->source[lexer->start + 1]) {
                    case 'f': 
                        if (lexer->current - lexer->start == 2) return TOKEN_IF;
                        break;
                    case 'n':
                        if (lexer->current - lexer->start == 3 &&
                            lexer->source[lexer->start + 2] == 't') return TOKEN_INT;
                        break;
                }
            }
            break;
        }
        case 'l':
            if (lexer->current - lexer->start == 4 &&
                strncmp(lexer->source + lexer->start + 1, "ist", 3) == 0)
                return TOKEN_LIST;
            else if (lexer->current - lexer->start == 4 &&
                strncmp(lexer->source + lexer->start + 1, "ong", 3) == 0)
                return TOKEN_LONG;
            else if (lexer->current - lexer->start == 6 &&
                strncmp(lexer->source + lexer->start + 1, "ength", 5) == 0)
                return TOKEN_LENGTH;
            break;
        case 'r':
            if (lexer->current - lexer->start == 6 &&
                strncmp(lexer->source + lexer->start + 1, "eturn", 5) == 0)
                return TOKEN_RETURN;
            else if (lexer->current - lexer->start == 6 &&
                strncmp(lexer->source + lexer->start + 1, "emove", 5) == 0)
                return TOKEN_REMOVE;
            break;
        case 's':
            if (lexer->current - lexer->start == 6 &&
                strncmp(lexer->source + lexer->start + 1, "tring", 5) == 0)
                return TOKEN_STRING;
            break;
        case 'v':
            if (lexer->current - lexer->start == 4 &&
                strncmp(lexer->source + lexer->start + 1, "oid", 3) == 0)
                return TOKEN_VOID;
            break;
        case 'w':
            if (lexer->current - lexer->start == 5 &&
                strncmp(lexer->source + lexer->start + 1, "hile", 4) == 0)
                return TOKEN_WHILE;
            break;
    }
    
    return TOKEN_IDENTIFIER;
}

void lexer_init(Lexer* lexer, const char* source) {
    lexer->source = source;
    lexer->start = 0;
    lexer->current = 0;
    lexer->line = 1;
    lexer->column = 0;
}

Token lexer_next_token(Lexer* lexer) {
    skip_whitespace(lexer);
    
    lexer->start = lexer->current;
    
    if (is_at_end(lexer)) return make_token(lexer, TOKEN_EOF);
    
    char c = advance(lexer);
    
    if (is_alpha(c)) {
        // Identifier
        while (is_alpha(peek(lexer)) || is_digit(peek(lexer))) advance(lexer);
        
        // Check for boolean literals
        TokenType type = identifier_type(lexer);
        
        // Special handling for "true" and "false" boolean literals
        if (type == TOKEN_IDENTIFIER) {
            if (lexer->current - lexer->start == 4 && strncmp(lexer->source + lexer->start, "true", 4) == 0)
                return make_token(lexer, TOKEN_BOOL_LITERAL);
            else if (lexer->current - lexer->start == 5 && strncmp(lexer->source + lexer->start, "false", 5) == 0)
                return make_token(lexer, TOKEN_BOOL_LITERAL);
        }
        
        return make_token(lexer, type);
    }
    
    if (is_digit(c)) {
        // Number
        return number(lexer);
    }
    
    switch (c) {
        case '(': return make_token(lexer, TOKEN_LPAREN);
        case ')': return make_token(lexer, TOKEN_RPAREN);
        case '{': return make_token(lexer, TOKEN_LBRACE);
        case '}': return make_token(lexer, TOKEN_RBRACE);
        case '[': return make_token(lexer, TOKEN_LBRACKET);
        case ']': return make_token(lexer, TOKEN_RBRACKET);
        case ';': return make_token(lexer, TOKEN_SEMICOLON);
        case ',': return make_token(lexer, TOKEN_COMMA);
        case '.': return make_token(lexer, TOKEN_DOT);
        case '-': return make_token(lexer, TOKEN_MINUS);
        case '+': return make_token(lexer, TOKEN_PLUS);
        case '/': return make_token(lexer, TOKEN_DIVIDE);
        case '*': return make_token(lexer, TOKEN_MULTIPLY);
        case '%': return make_token(lexer, TOKEN_MODULO);
        case '!': 
            return make_token(lexer, match(lexer, '=') ? TOKEN_NOT_EQUALS : TOKEN_BANG);
        case '=': 
            return make_token(lexer, match(lexer, '=') ? TOKEN_EQUALS : TOKEN_ASSIGN);
        case '<': 
            return make_token(lexer, match(lexer, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>': 
            return make_token(lexer, match(lexer, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
        case '"': return string(lexer);
    }
    
    return error_token(lexer, "Unexpected character.");
}

Token lexer_peek_token(Lexer* lexer) {
    int current_pos = lexer->current;
    int current_line = lexer->line;
    int current_column = lexer->column;
    
    Token token = lexer_next_token(lexer);
    
    lexer->current = current_pos;
    lexer->line = current_line;
    lexer->column = current_column;
    
    return token;
} 
