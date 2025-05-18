#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "parser.h"

// Forward declarations
static Expr* parse_expression(Parser* parser);
static Expr* parse_equality(Parser* parser);
static Expr* parse_comparison(Parser* parser);
static Expr* parse_term(Parser* parser);
static Expr* parse_factor(Parser* parser);
static Expr* parse_unary(Parser* parser);
static Expr* parse_primary(Parser* parser);
static Stmt* parse_statement(Parser* parser);
static Stmt* parse_declaration(Parser* parser);

static void advance(Parser* parser) {
    parser->previous = parser->current;
    
    for (;;) {
        parser->current = lexer_next_token(parser->lexer);
        if (parser->current.type != TOKEN_ERROR) break;
        
        parser_error_at_current(parser, parser->current.lexeme);
    }
}

static void consume(Parser* parser, TokenType type, const char* message) {
    if (parser->current.type == type) {
        advance(parser);
        return;
    }
    
    parser_error_at_current(parser, message);
}

static bool check(Parser* parser, TokenType type) {
    return parser->current.type == type;
}

static bool match(Parser* parser, TokenType type) {
    if (!check(parser, type)) return false;
    advance(parser);
    return true;
}

static DataType parse_type(Parser* parser) {
    if (match(parser, TOKEN_INT)) return TYPE_INT;
    if (match(parser, TOKEN_FLOAT)) return TYPE_FLOAT;
    if (match(parser, TOKEN_STRING)) return TYPE_STRING;
    if (match(parser, TOKEN_VOID)) return TYPE_VOID;
    
    parser_error_at_current(parser, "Expected type.");
    return TYPE_VOID; // Error recovery
}

static Expr* parse_primary(Parser* parser) {
    if (match(parser, TOKEN_INTEGER_LITERAL) ||
        match(parser, TOKEN_FLOAT_LITERAL) ||
        match(parser, TOKEN_STRING_LITERAL)) {
        Token* token = malloc(sizeof(Token));
        *token = parser->previous;
        return create_literal_expr(token);
    }
    
    if (match(parser, TOKEN_IDENTIFIER)) {
        Token name = parser->previous;
        
        if (match(parser, TOKEN_LPAREN)) {
            // Function call
            Expr** arguments = NULL;
            int arg_count = 0;
            
            if (!check(parser, TOKEN_RPAREN)) {
                do {
                    if (arg_count >= 255) {
                        parser_error_at_current(parser, "Cannot have more than 255 arguments.");
                    }
                    
                    arguments = realloc(arguments, sizeof(Expr*) * (arg_count + 1));
                    arguments[arg_count++] = parse_expression(parser);
                } while (match(parser, TOKEN_COMMA));
            }
            
            consume(parser, TOKEN_RPAREN, "Expect ')' after arguments.");
            
            return create_call_expr(create_variable_expr(name, TYPE_VOID),
                                  arguments, arg_count);
        }
        
        return create_variable_expr(name, TYPE_VOID);
    }
    
    if (match(parser, TOKEN_LPAREN)) {
        Expr* expr = parse_expression(parser);
        consume(parser, TOKEN_RPAREN, "Expect ')' after expression.");
        return expr;
    }
    
    parser_error_at_current(parser, "Expect expression.");
    return NULL;
}

static Expr* parse_unary(Parser* parser) {
    if (match(parser, TOKEN_MINUS)) {
        Token operator = parser->previous;
        Expr* right = parse_unary(parser);
        return create_unary_expr(operator, right);
    }
    
    return parse_primary(parser);
}

static Expr* parse_factor(Parser* parser) {
    Expr* expr = parse_unary(parser);
    
    while (match(parser, TOKEN_MULTIPLY) || match(parser, TOKEN_DIVIDE) || match(parser, TOKEN_MODULO)) {
        Token operator = parser->previous;
        Expr* right = parse_unary(parser);
        expr = create_binary_expr(operator, expr, right);
    }
    
    return expr;
}

static Expr* parse_term(Parser* parser) {
    Expr* expr = parse_factor(parser);
    
    while (match(parser, TOKEN_PLUS) || match(parser, TOKEN_MINUS)) {
        Token operator = parser->previous;
        Expr* right = parse_factor(parser);
        expr = create_binary_expr(operator, expr, right);
    }
    
    return expr;
}

static Expr* parse_comparison(Parser* parser) {
    Expr* expr = parse_term(parser);
    
    while (match(parser, TOKEN_LESS) || match(parser, TOKEN_LESS_EQUAL) ||
           match(parser, TOKEN_GREATER) || match(parser, TOKEN_GREATER_EQUAL)) {
        Token operator = parser->previous;
        Expr* right = parse_term(parser);
        expr = create_binary_expr(operator, expr, right);
    }
    
    return expr;
}

static Expr* parse_equality(Parser* parser) {
    Expr* expr = parse_comparison(parser);
    
    while (match(parser, TOKEN_EQUALS) || match(parser, TOKEN_NOT_EQUALS)) {
        Token operator = parser->previous;
        Expr* right = parse_comparison(parser);
        expr = create_binary_expr(operator, expr, right);
    }
    
    return expr;
}

static Expr* parse_assignment(Parser* parser) {
    Expr* expr = parse_equality(parser);
    
    if (match(parser, TOKEN_ASSIGN)) {
        Token equals = parser->previous;
        Expr* value = parse_assignment(parser);
        
        if (expr->type == EXPR_VARIABLE) {
            Token name = expr->as.variable.name;
            return create_assign_expr(name, value);
        }
        
        parser_error_at_previous(parser, "Invalid assignment target.");
    }
    
    return expr;
}

static Expr* parse_expression(Parser* parser) {
    return parse_assignment(parser);
}

static Stmt* parse_var_declaration(Parser* parser) {
    DataType type = parse_type(parser);
    Token name = parser->current;
    consume(parser, TOKEN_IDENTIFIER, "Expect variable name.");
    
    Expr* initializer = NULL;
    if (match(parser, TOKEN_ASSIGN)) {
        initializer = parse_expression(parser);
    }
    
    // Create the first variable declaration
    Stmt* first = create_var_decl_stmt(name, type, initializer);
    
    // If there's a comma, parse additional variable declarations
    if (match(parser, TOKEN_COMMA)) {
        // Create a block to hold all declarations
        Stmt** statements = malloc(sizeof(Stmt*));
        int count = 1;
        statements[0] = first;
        
        do {
            name = parser->current;
            consume(parser, TOKEN_IDENTIFIER, "Expect variable name.");
            
            initializer = NULL;
            if (match(parser, TOKEN_ASSIGN)) {
                initializer = parse_expression(parser);
            }
            
            statements = realloc(statements, sizeof(Stmt*) * (count + 1));
            statements[count++] = create_var_decl_stmt(name, type, initializer);
        } while (match(parser, TOKEN_COMMA));
        
        consume(parser, TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
        return create_block_stmt(statements, count);
    }
    
    consume(parser, TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
    return first;
}

static Stmt* parse_statement(Parser* parser);

static Stmt* parse_block(Parser* parser) {
    Stmt** statements = NULL;
    int count = 0;
    
    while (!check(parser, TOKEN_RBRACE) && !check(parser, TOKEN_EOF)) {
        statements = realloc(statements, sizeof(Stmt*) * (count + 1));
        statements[count++] = parse_statement(parser);
    }
    
    consume(parser, TOKEN_RBRACE, "Expect '}' after block.");
    return create_block_stmt(statements, count);
}

static Stmt* parse_if_statement(Parser* parser) {
    consume(parser, TOKEN_LPAREN, "Expect '(' after 'if'.");
    Expr* condition = parse_expression(parser);
    consume(parser, TOKEN_RPAREN, "Expect ')' after condition.");
    
    Stmt* then_branch = parse_statement(parser);
    Stmt* else_branch = NULL;
    
    if (match(parser, TOKEN_ELSE)) {
        else_branch = parse_statement(parser);
    }
    
    return create_if_stmt(condition, then_branch, else_branch);
}

static Stmt* parse_while_statement(Parser* parser) {
    consume(parser, TOKEN_LPAREN, "Expect '(' after 'while'.");
    Expr* condition = parse_expression(parser);
    consume(parser, TOKEN_RPAREN, "Expect ')' after condition.");
    Stmt* body = parse_statement(parser);
    
    return create_while_stmt(condition, body);
}

static Stmt* parse_return_statement(Parser* parser) {
    Expr* value = NULL;
    if (!check(parser, TOKEN_SEMICOLON)) {
        value = parse_expression(parser);
    }
    
    consume(parser, TOKEN_SEMICOLON, "Expect ';' after return value.");
    return create_return_stmt(value);
}

static Stmt* parse_statement(Parser* parser) {
    if (match(parser, TOKEN_IF)) return parse_if_statement(parser);
    if (match(parser, TOKEN_WHILE)) return parse_while_statement(parser);
    if (match(parser, TOKEN_RETURN)) return parse_return_statement(parser);
    if (match(parser, TOKEN_LBRACE)) return parse_block(parser);
    
    if (check(parser, TOKEN_INT) || check(parser, TOKEN_FLOAT) ||
        check(parser, TOKEN_STRING) || check(parser, TOKEN_VOID)) {
        return parse_var_declaration(parser);
    }
    
    Stmt* stmt = create_expression_stmt(parse_expression(parser));
    consume(parser, TOKEN_SEMICOLON, "Expect ';' after expression.");
    return stmt;
}

static Stmt* parse_declaration(Parser* parser) {
    if (check(parser, TOKEN_INT) || check(parser, TOKEN_FLOAT) ||
        check(parser, TOKEN_STRING) || check(parser, TOKEN_VOID)) {
        DataType type = parse_type(parser);
        Token name = parser->current;
        consume(parser, TOKEN_IDENTIFIER, "Expect identifier.");
        
        if (match(parser, TOKEN_LPAREN)) {
            // Function declaration
            Token* parameters = NULL;
            DataType* param_types = NULL;
            int param_count = 0;
            
            if (!check(parser, TOKEN_RPAREN)) {
                do {
                    if (param_count >= 255) {
                        parser_error_at_current(parser, "Cannot have more than 255 parameters.");
                    }
                    
                    DataType param_type = parse_type(parser);
                    Token param_name = parser->current;
                    consume(parser, TOKEN_IDENTIFIER, "Expect parameter name.");
                    
                    parameters = realloc(parameters, sizeof(Token) * (param_count + 1));
                    param_types = realloc(param_types, sizeof(DataType) * (param_count + 1));
                    
                    parameters[param_count] = param_name;
                    param_types[param_count] = param_type;
                    param_count++;
                } while (match(parser, TOKEN_COMMA));
            }
            
            consume(parser, TOKEN_RPAREN, "Expect ')' after parameters.");
            consume(parser, TOKEN_LBRACE, "Expect '{' before function body.");
            Stmt* body = parse_block(parser);
            
            return create_function_stmt(name, type, parameters, param_types,
                                     param_count, body);
        } else {
            // Variable declaration
            Expr* initializer = NULL;
            if (match(parser, TOKEN_ASSIGN)) {
                initializer = parse_expression(parser);
            }
            
            consume(parser, TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
            return create_var_decl_stmt(name, type, initializer);
        }
    }
    
    return parse_statement(parser);
}

void parser_init(Parser* parser, Lexer* lexer) {
    parser->lexer = lexer;
    parser->had_error = false;
    parser->panic_mode = false;
    advance(parser);
}

Stmt** parse(Parser* parser, int* count) {
    Stmt** statements = NULL;
    *count = 0;
    
    while (!check(parser, TOKEN_EOF)) {
        statements = realloc(statements, sizeof(Stmt*) * (*count + 1));
        statements[*count] = parse_declaration(parser);
        (*count)++;
    }
    
    return statements;
}

void parser_error_at_current(Parser* parser, const char* message) {
    if (parser->panic_mode) return;
    parser->panic_mode = true;
    parser->had_error = true;
    
    fprintf(stderr, "[line %d] Error at '%s': %s\n",
            parser->current.line,
            parser->current.lexeme,
            message);
}

void parser_error_at_previous(Parser* parser, const char* message) {
    if (parser->panic_mode) return;
    parser->panic_mode = true;
    parser->had_error = true;
    
    fprintf(stderr, "[line %d] Error at '%s': %s\n",
            parser->previous.line,
            parser->previous.lexeme,
            message);
} 