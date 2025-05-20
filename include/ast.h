#ifndef AST_H
#define AST_H

#include <stdlib.h>

// Forward declarations
typedef struct ASTNode ASTNode;

// AST node types
typedef enum {
    AST_PROGRAM,
    AST_INT,
    AST_FLOAT,
    AST_STRING,
    AST_IDENTIFIER,
    AST_VAR_DECL,
    AST_ASSIGN,
    AST_BINOP,
    AST_IF,
    AST_WHILE,
    AST_FOR,
    AST_FOREACH,
    AST_FUNC_DEF,
    AST_FUNC_CALL,
    AST_RETURN,
    AST_BLOCK,
    AST_STRUCT_DEF,
    AST_STRUCT_ACCESS,
    AST_LIST,
    AST_INCLUDE
} ASTNodeType;

// Generic AST node structure
struct ASTNode {
    ASTNodeType type;
    union {
        // AST_INT
        int int_value;
        
        // AST_FLOAT
        double float_value;
        
        // AST_STRING, AST_IDENTIFIER
        char* string_value;
        
        // AST_VAR_DECL
        struct {
            int var_type;
            char* var_name;
            ASTNode* var_value;
        };
        
        // AST_ASSIGN
        struct {
            ASTNode* assign_left;
            ASTNode* assign_right;
        };
        
        // AST_BINOP
        struct {
            ASTNode* left;
            int op;
            ASTNode* right;
        };
        
        // AST_IF
        struct {
            ASTNode* if_condition;
            ASTNode* if_body;
            ASTNode* else_body;
        };
        
        // AST_WHILE, AST_FOR
        struct {
            ASTNode* loop_init;
            ASTNode* loop_condition;
            ASTNode* loop_update;
            ASTNode* loop_body;
        };
        
        // AST_FOREACH
        struct {
            int foreach_var_type;
            char* foreach_var_name;
            ASTNode* foreach_list;
            ASTNode* foreach_body;
        };
        
        // AST_FUNC_DEF
        struct {
            int func_return_type;
            char* func_name;
            ASTNode** func_params;
            int func_param_count;
            ASTNode* func_body;
        };
        
        // AST_FUNC_CALL
        struct {
            char* call_name;
            ASTNode** call_args;
            int call_arg_count;
        };
        
        // AST_RETURN
        ASTNode* return_value;
        
        // AST_BLOCK, AST_PROGRAM
        struct {
            ASTNode** block_statements;
            int block_statement_count;
        };
        
        // AST_STRUCT_DEF
        struct {
            char* struct_name;
            char** struct_field_names;
            int* struct_field_types;
            int struct_field_count;
        };
        
        // AST_STRUCT_ACCESS
        struct {
            ASTNode* struct_var;
            char* struct_field;
        };
        
        // AST_LIST
        struct {
            ASTNode** list_elements;
            int list_element_count;
        };
        
        // AST_INCLUDE
        char* include_path;
    };
};

// Function prototypes
ASTNode* ast_create_node(ASTNodeType type);
void ast_free_node(ASTNode* node);

#endif /* AST_H */ 