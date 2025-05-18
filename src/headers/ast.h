#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include "token.h"

typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_VOID
} DataType;

typedef enum {
    EXPR_BINARY,
    EXPR_UNARY,
    EXPR_LITERAL,
    EXPR_VARIABLE,
    EXPR_CALL,
    EXPR_ASSIGN
} ExprType;

typedef enum {
    STMT_EXPRESSION,
    STMT_VAR_DECL,
    STMT_BLOCK,
    STMT_IF,
    STMT_WHILE,
    STMT_RETURN,
    STMT_FUNCTION
} StmtType;

// Forward declarations
typedef struct Expr Expr;
typedef struct Stmt Stmt;

// Expression structures
typedef struct {
    Token operator;
    Expr* left;
    Expr* right;
} BinaryExpr;

typedef struct {
    Token operator;
    Expr* operand;
} UnaryExpr;

typedef struct {
    Token name;
    DataType type;
} VariableExpr;

typedef struct {
    Token* value;
} LiteralExpr;

typedef struct {
    Expr* callee;
    Expr** arguments;
    int arg_count;
} CallExpr;

typedef struct {
    Token name;
    Expr* value;
} AssignExpr;

struct Expr {
    ExprType type;
    union {
        BinaryExpr binary;
        UnaryExpr unary;
        VariableExpr variable;
        LiteralExpr literal;
        CallExpr call;
        AssignExpr assign;
    } as;
};

// Statement structures
typedef struct {
    Token name;
    DataType return_type;
    Token* params;
    DataType* param_types;
    int param_count;
    Stmt* body;
} FunctionStmt;

typedef struct {
    Token name;
    DataType type;
    Expr* initializer;
} VarDeclStmt;

typedef struct {
    Expr* condition;
    Stmt* then_branch;
    Stmt* else_branch;
} IfStmt;

typedef struct {
    Expr* condition;
    Stmt* body;
} WhileStmt;

typedef struct {
    Stmt** statements;
    int count;
} BlockStmt;

typedef struct {
    Expr* expression;
} ReturnStmt;

struct Stmt {
    StmtType type;
    union {
        Expr* expression;
        VarDeclStmt var_decl;
        BlockStmt block;
        IfStmt if_stmt;
        WhileStmt while_stmt;
        ReturnStmt return_stmt;
        FunctionStmt function;
    } as;
};

// AST node creation functions
Expr* create_binary_expr(Token operator, Expr* left, Expr* right);
Expr* create_unary_expr(Token operator, Expr* operand);
Expr* create_literal_expr(Token* value);
Expr* create_variable_expr(Token name, DataType type);
Expr* create_call_expr(Expr* callee, Expr** arguments, int arg_count);
Expr* create_assign_expr(Token name, Expr* value);

Stmt* create_expression_stmt(Expr* expression);
Stmt* create_var_decl_stmt(Token name, DataType type, Expr* initializer);
Stmt* create_block_stmt(Stmt** statements, int count);
Stmt* create_if_stmt(Expr* condition, Stmt* then_branch, Stmt* else_branch);
Stmt* create_while_stmt(Expr* condition, Stmt* body);
Stmt* create_return_stmt(Expr* expression);
Stmt* create_function_stmt(Token name, DataType return_type, Token* params, DataType* param_types, int param_count, Stmt* body);

// Memory management functions
void free_expr(Expr* expr);
void free_stmt(Stmt* stmt);

// AST debugging functions
void print_expr(Expr* expr, int indent);
void print_stmt(Stmt* stmt, int indent);
void print_ast(Stmt** statements, int count);

#endif // AST_H 