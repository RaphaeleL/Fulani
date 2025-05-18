#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "lexer.h"
#include "ast.h"

typedef struct {
    Lexer* lexer;
    Token current;
    Token previous;
    bool had_error;
    bool panic_mode;
} Parser;

void parser_init(Parser* parser, Lexer* lexer);
Stmt** parse(Parser* parser, int* count);
void parser_error_at_current(Parser* parser, const char* message);
void parser_error_at_previous(Parser* parser, const char* message);

#endif // PARSER_H 