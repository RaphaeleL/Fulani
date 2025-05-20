#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct {
    Lexer* lexer;
    Token current_token;
} Parser;

Parser* parser_init(Lexer* lexer);
void parser_free(Parser* parser);
void parser_eat(Parser* parser, TokenType token_type);
ASTNode* parser_parse(Parser* parser);

#endif /* PARSER_H */ 