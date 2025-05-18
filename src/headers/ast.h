#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include "token.h"

typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_VOID,
    TYPE_BOOL,    // Boolean type
    TYPE_LIST,    // List type
    TYPE_DOUBLE,  // Double type
    TYPE_LONG     // Long type
} DataType;

typedef enum {
    EXPR_BINARY,
    EXPR_UNARY,
    EXPR_LITERAL,
    EXPR_VARIABLE,
    EXPR_CALL,
    EXPR_ASSIGN,
    EXPR_LIST_ACCESS,     // For list[index]
    EXPR_LIST_METHOD,     // For list.add(item) or list.remove(index)
    EXPR_LIST_PROPERTY    // For list.length
} ExprType;

typedef enum {
    STMT_EXPRESSION,
    STMT_VAR_DECL,
    STMT_BLOCK,
    STMT_IF,
    STMT_WHILE,
    STMT_FOR,
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

// New list access expression (list[index])
typedef struct {
    Expr* list;
    Expr* index;
} ListAccessExpr;

// New list method expression (list.add(item) or list.remove(index))
typedef struct {
    Expr* list;
    TokenType method;
    Expr* argument;
} ListMethodExpr;

// New list property expression (list.length)
typedef struct {
    Expr* list;
    TokenType property;
} ListPropertyExpr;

struct Expr {
    ExprType type;
    union {
        BinaryExpr binary;
        UnaryExpr unary;
        VariableExpr variable;
        LiteralExpr literal;
        CallExpr call;
        AssignExpr assign;
        ListAccessExpr list_access;
        ListMethodExpr list_method;
        ListPropertyExpr list_property;
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
    Stmt* init;        // Initialization (var declaration or expression)
    Expr* condition;   // Loop condition
    Expr* increment;   // Increment expression
    Stmt* body;        // Loop body
} ForStmt;

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
        ForStmt for_stmt;
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
Expr* create_list_access_expr(Expr* list, Expr* index);
Expr* create_list_method_expr(Expr* list, TokenType method, Expr* argument);
Expr* create_list_property_expr(Expr* list, TokenType property);

Stmt* create_expression_stmt(Expr* expression);
Stmt* create_var_decl_stmt(Token name, DataType type, Expr* initializer);
Stmt* create_block_stmt(Stmt** statements, int count);
Stmt* create_if_stmt(Expr* condition, Stmt* then_branch, Stmt* else_branch);
Stmt* create_while_stmt(Expr* condition, Stmt* body);
Stmt* create_for_stmt(Stmt* init, Expr* condition, Expr* increment, Stmt* body);
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