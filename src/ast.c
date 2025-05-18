#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "headers/ast.h"

// AST debugging and pretty printing functions
static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

static const char* datatype_to_string(DataType type) {
    switch (type) {
        case TYPE_INT: return "int";
        case TYPE_FLOAT: return "float";
        case TYPE_STRING: return "string";
        case TYPE_VOID: return "void";
        default: return "unknown";
    }
}

static const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_INT: return "INT";
        case TOKEN_FLOAT: return "FLOAT";
        case TOKEN_STRING: return "STRING";
        case TOKEN_VOID: return "VOID";
        case TOKEN_PLUS: return "+";
        case TOKEN_MINUS: return "-";
        case TOKEN_MULTIPLY: return "*";
        case TOKEN_DIVIDE: return "/";
        case TOKEN_EQUALS: return "==";
        case TOKEN_NOT_EQUALS: return "!=";
        case TOKEN_LESS: return "<";
        case TOKEN_LESS_EQUAL: return "<=";
        case TOKEN_GREATER: return ">";
        case TOKEN_GREATER_EQUAL: return ">=";
        case TOKEN_ASSIGN: return "=";
        case TOKEN_COMMA: return ",";
        case TOKEN_SEMICOLON: return ";";
        case TOKEN_LPAREN: return "(";
        case TOKEN_RPAREN: return ")";
        case TOKEN_LBRACE: return "{";
        case TOKEN_RBRACE: return "}";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_INTEGER_LITERAL: return "INTEGER";
        case TOKEN_FLOAT_LITERAL: return "FLOAT";
        case TOKEN_STRING_LITERAL: return "STRING";
        case TOKEN_IF: return "if";
        case TOKEN_ELSE: return "else";
        case TOKEN_WHILE: return "while";
        case TOKEN_RETURN: return "return";
        case TOKEN_EOF: return "EOF";
        default: return "UNKNOWN";
    }
}

void print_expr(Expr* expr, int indent) {
    if (expr == NULL) {
        print_indent(indent);
        printf("<null>\n");
        return;
    }
    
    switch (expr->type) {
        case EXPR_LITERAL: {
            Token* token = expr->as.literal.value;
            print_indent(indent);
            printf("Literal(%s): %s\n", token_type_to_string(token->type), token->lexeme);
            break;
        }
        case EXPR_BINARY: {
            print_indent(indent);
            printf("Binary(%s):\n", token_type_to_string(expr->as.binary.operator.type));
            print_indent(indent + 1);
            printf("Left:\n");
            print_expr(expr->as.binary.left, indent + 2);
            print_indent(indent + 1);
            printf("Right:\n");
            print_expr(expr->as.binary.right, indent + 2);
            break;
        }
        case EXPR_UNARY: {
            print_indent(indent);
            printf("Unary(%s):\n", token_type_to_string(expr->as.unary.operator.type));
            print_indent(indent + 1);
            printf("Operand:\n");
            print_expr(expr->as.unary.operand, indent + 2);
            break;
        }
        case EXPR_VARIABLE: {
            print_indent(indent);
            printf("Variable(%s): %s\n", datatype_to_string(expr->as.variable.type), 
                   expr->as.variable.name.lexeme);
            break;
        }
        case EXPR_ASSIGN: {
            print_indent(indent);
            printf("Assign(%s):\n", expr->as.assign.name.lexeme);
            print_indent(indent + 1);
            printf("Value:\n");
            print_expr(expr->as.assign.value, indent + 2);
            break;
        }
        case EXPR_CALL: {
            print_indent(indent);
            printf("Call:\n");
            print_indent(indent + 1);
            printf("Callee:\n");
            print_expr(expr->as.call.callee, indent + 2);
            print_indent(indent + 1);
            printf("Arguments(%d):\n", expr->as.call.arg_count);
            for (int i = 0; i < expr->as.call.arg_count; i++) {
                print_indent(indent + 2);
                printf("Arg %d:\n", i);
                print_expr(expr->as.call.arguments[i], indent + 3);
            }
            break;
        }
    }
}

void print_stmt(Stmt* stmt, int indent) {
    if (stmt == NULL) {
        print_indent(indent);
        printf("<null>\n");
        return;
    }
    
    switch (stmt->type) {
        case STMT_EXPRESSION: {
            print_indent(indent);
            printf("Expression:\n");
            print_expr(stmt->as.expression, indent + 1);
            break;
        }
        case STMT_VAR_DECL: {
            print_indent(indent);
            printf("VarDecl(%s %s):\n", datatype_to_string(stmt->as.var_decl.type), 
                   stmt->as.var_decl.name.lexeme);
            if (stmt->as.var_decl.initializer != NULL) {
                print_indent(indent + 1);
                printf("Initializer:\n");
                print_expr(stmt->as.var_decl.initializer, indent + 2);
            }
            break;
        }
        case STMT_BLOCK: {
            print_indent(indent);
            printf("Block(%d statements):\n", stmt->as.block.count);
            for (int i = 0; i < stmt->as.block.count; i++) {
                print_stmt(stmt->as.block.statements[i], indent + 1);
            }
            break;
        }
        case STMT_IF: {
            print_indent(indent);
            printf("If:\n");
            print_indent(indent + 1);
            printf("Condition:\n");
            print_expr(stmt->as.if_stmt.condition, indent + 2);
            print_indent(indent + 1);
            printf("Then:\n");
            print_stmt(stmt->as.if_stmt.then_branch, indent + 2);
            if (stmt->as.if_stmt.else_branch != NULL) {
                print_indent(indent + 1);
                printf("Else:\n");
                print_stmt(stmt->as.if_stmt.else_branch, indent + 2);
            }
            break;
        }
        case STMT_WHILE: {
            print_indent(indent);
            printf("While:\n");
            print_indent(indent + 1);
            printf("Condition:\n");
            print_expr(stmt->as.while_stmt.condition, indent + 2);
            print_indent(indent + 1);
            printf("Body:\n");
            print_stmt(stmt->as.while_stmt.body, indent + 2);
            break;
        }
        case STMT_FUNCTION: {
            print_indent(indent);
            printf("Function(%s %s):\n", datatype_to_string(stmt->as.function.return_type), 
                   stmt->as.function.name.lexeme);
            print_indent(indent + 1);
            printf("Parameters(%d):\n", stmt->as.function.param_count);
            for (int i = 0; i < stmt->as.function.param_count; i++) {
                print_indent(indent + 2);
                printf("%s %s\n", datatype_to_string(stmt->as.function.param_types[i]), 
                       stmt->as.function.params[i].lexeme);
            }
            print_indent(indent + 1);
            printf("Body:\n");
            print_stmt(stmt->as.function.body, indent + 2);
            break;
        }
        case STMT_RETURN: {
            print_indent(indent);
            printf("Return:\n");
            if (stmt->as.return_stmt.expression != NULL) {
                print_expr(stmt->as.return_stmt.expression, indent + 1);
            }
            break;
        }
    }
}

void print_ast(Stmt** statements, int count) {
    printf("\n===== AST DUMP =====\n\n");
    for (int i = 0; i < count; i++) {
        printf("Statement %d:\n", i);
        print_stmt(statements[i], 1);
        printf("\n");
    }
    printf("===== END AST DUMP =====\n\n");
}

// Expression creation functions
Expr* create_binary_expr(Token op, Expr* left, Expr* right) {
    Expr* expr = (Expr*)malloc(sizeof(Expr));
    expr->type = EXPR_BINARY;
    expr->as.binary.operator = op;
    expr->as.binary.left = left;
    expr->as.binary.right = right;
    return expr;
}

Expr* create_unary_expr(Token op, Expr* operand) {
    Expr* expr = (Expr*)malloc(sizeof(Expr));
    expr->type = EXPR_UNARY;
    expr->as.unary.operator = op;
    expr->as.unary.operand = operand;
    return expr;
}

Expr* create_literal_expr(Token* value) {
    Expr* expr = (Expr*)malloc(sizeof(Expr));
    expr->type = EXPR_LITERAL;
    expr->as.literal.value = value;
    return expr;
}

Expr* create_variable_expr(Token name, DataType type) {
    Expr* expr = (Expr*)malloc(sizeof(Expr));
    expr->type = EXPR_VARIABLE;
    expr->as.variable.name = name;
    expr->as.variable.type = type;
    return expr;
}

Expr* create_call_expr(Expr* callee, Expr** arguments, int arg_count) {
    Expr* expr = (Expr*)malloc(sizeof(Expr));
    expr->type = EXPR_CALL;
    expr->as.call.callee = callee;
    expr->as.call.arguments = arguments;
    expr->as.call.arg_count = arg_count;
    return expr;
}

Expr* create_assign_expr(Token name, Expr* value) {
    Expr* expr = (Expr*)malloc(sizeof(Expr));
    expr->type = EXPR_ASSIGN;
    expr->as.assign.name = name;
    expr->as.assign.value = value;
    return expr;
}

// Statement creation functions
Stmt* create_expression_stmt(Expr* expression) {
    Stmt* stmt = (Stmt*)malloc(sizeof(Stmt));
    stmt->type = STMT_EXPRESSION;
    stmt->as.expression = expression;
    return stmt;
}

Stmt* create_var_decl_stmt(Token name, DataType type, Expr* initializer) {
    Stmt* stmt = (Stmt*)malloc(sizeof(Stmt));
    stmt->type = STMT_VAR_DECL;
    stmt->as.var_decl.name = name;
    stmt->as.var_decl.type = type;
    stmt->as.var_decl.initializer = initializer;
    return stmt;
}

Stmt* create_block_stmt(Stmt** statements, int count) {
    Stmt* stmt = (Stmt*)malloc(sizeof(Stmt));
    stmt->type = STMT_BLOCK;
    stmt->as.block.statements = statements;
    stmt->as.block.count = count;
    return stmt;
}

Stmt* create_if_stmt(Expr* condition, Stmt* then_branch, Stmt* else_branch) {
    Stmt* stmt = (Stmt*)malloc(sizeof(Stmt));
    stmt->type = STMT_IF;
    stmt->as.if_stmt.condition = condition;
    stmt->as.if_stmt.then_branch = then_branch;
    stmt->as.if_stmt.else_branch = else_branch;
    return stmt;
}

Stmt* create_while_stmt(Expr* condition, Stmt* body) {
    Stmt* stmt = (Stmt*)malloc(sizeof(Stmt));
    stmt->type = STMT_WHILE;
    stmt->as.while_stmt.condition = condition;
    stmt->as.while_stmt.body = body;
    return stmt;
}

Stmt* create_return_stmt(Expr* expression) {
    Stmt* stmt = (Stmt*)malloc(sizeof(Stmt));
    stmt->type = STMT_RETURN;
    stmt->as.return_stmt.expression = expression;
    return stmt;
}

Stmt* create_function_stmt(Token name, DataType return_type, Token* params,
                          DataType* param_types, int param_count, Stmt* body) {
    Stmt* stmt = (Stmt*)malloc(sizeof(Stmt));
    stmt->type = STMT_FUNCTION;
    stmt->as.function.name = name;
    stmt->as.function.return_type = return_type;
    stmt->as.function.params = params;
    stmt->as.function.param_types = param_types;
    stmt->as.function.param_count = param_count;
    stmt->as.function.body = body;
    return stmt;
}

// Memory management functions
void free_expr(Expr* expr) {
    if (expr == NULL) return;
    
    switch (expr->type) {
        case EXPR_BINARY:
            free_expr(expr->as.binary.left);
            free_expr(expr->as.binary.right);
            break;
        case EXPR_UNARY:
            free_expr(expr->as.unary.operand);
            break;
        case EXPR_CALL:
            free_expr(expr->as.call.callee);
            for (int i = 0; i < expr->as.call.arg_count; i++) {
                free_expr(expr->as.call.arguments[i]);
            }
            free(expr->as.call.arguments);
            break;
        case EXPR_ASSIGN:
            free_expr(expr->as.assign.value);
            break;
        default:
            break;
    }
    
    free(expr);
}

void free_stmt(Stmt* stmt) {
    if (stmt == NULL) return;
    
    switch (stmt->type) {
        case STMT_EXPRESSION:
            free_expr(stmt->as.expression);
            break;
        case STMT_VAR_DECL:
            free_expr(stmt->as.var_decl.initializer);
            break;
        case STMT_BLOCK:
            for (int i = 0; i < stmt->as.block.count; i++) {
                free_stmt(stmt->as.block.statements[i]);
            }
            free(stmt->as.block.statements);
            break;
        case STMT_IF:
            free_expr(stmt->as.if_stmt.condition);
            free_stmt(stmt->as.if_stmt.then_branch);
            free_stmt(stmt->as.if_stmt.else_branch);
            break;
        case STMT_WHILE:
            free_expr(stmt->as.while_stmt.condition);
            free_stmt(stmt->as.while_stmt.body);
            break;
        case STMT_RETURN:
            free_expr(stmt->as.return_stmt.expression);
            break;
        case STMT_FUNCTION:
            free(stmt->as.function.params);
            free(stmt->as.function.param_types);
            free_stmt(stmt->as.function.body);
            break;
    }
    
    free(stmt);
} 
