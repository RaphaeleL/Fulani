#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/utils.h"

// Helper function to print AST (for debugging)
void print_ast(ASTNode* node, int depth) {
    if (node == NULL) {
        return;
    }
    
    // Print indentation
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
    
    // Print node type
    switch (node->type) {
        case AST_PROGRAM:
            printf("PROGRAM (%d statements)\n", node->block_statement_count);
            for (int i = 0; i < node->block_statement_count; i++) {
                print_ast(node->block_statements[i], depth + 1);
            }
            break;
            
        case AST_BLOCK:
            printf("BLOCK (%d statements)\n", node->block_statement_count);
            for (int i = 0; i < node->block_statement_count; i++) {
                print_ast(node->block_statements[i], depth + 1);
            }
            break;
            
        case AST_VAR_DECL:
            printf("VAR_DECL (type: %d, name: %s)\n", node->var_type, node->var_name);
            if (node->var_value) {
                print_ast(node->var_value, depth + 1);
            }
            break;
            
        case AST_ASSIGN:
            printf("ASSIGN\n");
            print_ast(node->assign_left, depth + 1);
            print_ast(node->assign_right, depth + 1);
            break;
            
        case AST_BINOP:
            printf("BINOP (op: %d)\n", node->op);
            print_ast(node->left, depth + 1);
            print_ast(node->right, depth + 1);
            break;
            
        case AST_IF:
            printf("IF\n");
            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("CONDITION:\n");
            print_ast(node->if_condition, depth + 2);
            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("THEN:\n");
            print_ast(node->if_body, depth + 2);
            if (node->else_body) {
                for (int i = 0; i < depth + 1; i++) printf("  ");
                printf("ELSE:\n");
                print_ast(node->else_body, depth + 2);
            }
            break;
            
        case AST_WHILE:
            printf("WHILE\n");
            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("CONDITION:\n");
            print_ast(node->loop_condition, depth + 2);
            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("BODY:\n");
            print_ast(node->loop_body, depth + 2);
            break;
            
        case AST_FOR:
            printf("FOR\n");
            if (node->loop_init) {
                for (int i = 0; i < depth + 1; i++) printf("  ");
                printf("INIT:\n");
                print_ast(node->loop_init, depth + 2);
            }
            if (node->loop_condition) {
                for (int i = 0; i < depth + 1; i++) printf("  ");
                printf("CONDITION:\n");
                print_ast(node->loop_condition, depth + 2);
            }
            if (node->loop_update) {
                for (int i = 0; i < depth + 1; i++) printf("  ");
                printf("UPDATE:\n");
                print_ast(node->loop_update, depth + 2);
            }
            for (int i = 0; i < depth + 1; i++) printf("  ");
            printf("BODY:\n");
            print_ast(node->loop_body, depth + 2);
            break;
            
        case AST_FUNC_DEF:
            printf("FUNC_DEF (name: %s, return_type: %d, %d params)\n", 
                  node->func_name, node->func_return_type, node->func_param_count);
            for (int i = 0; i < node->func_param_count; i++) {
                print_ast(node->func_params[i], depth + 1);
            }
            print_ast(node->func_body, depth + 1);
            break;
            
        case AST_FUNC_CALL:
            printf("FUNC_CALL (name: %s, %d args)\n", node->call_name, node->call_arg_count);
            for (int i = 0; i < node->call_arg_count; i++) {
                print_ast(node->call_args[i], depth + 1);
            }
            break;
            
        case AST_RETURN:
            printf("RETURN\n");
            if (node->return_value) {
                print_ast(node->return_value, depth + 1);
            }
            break;
            
        case AST_INT:
            printf("INT (%d)\n", node->int_value);
            break;
            
        case AST_FLOAT:
            printf("FLOAT (%f)\n", node->float_value);
            break;
            
        case AST_STRING:
            printf("STRING (\"%s\")\n", node->string_value);
            break;
            
        case AST_IDENTIFIER:
            printf("IDENTIFIER (%s)\n", node->string_value);
            break;
            
        case AST_INCLUDE:
            printf("INCLUDE (%s)\n", node->include_path);
            break;
            
        case AST_STRUCT_DEF:
            printf("STRUCT_DEF (name: %s, %d fields)\n", node->struct_name, node->struct_field_count);
            for (int i = 0; i < node->struct_field_count; i++) {
                for (int j = 0; j < depth + 1; j++) printf("  ");
                printf("FIELD: %s (type: %d)\n", node->struct_field_names[i], node->struct_field_types[i]);
            }
            break;
            
        default:
            printf("UNKNOWN_NODE_TYPE (%d)\n", node->type);
            break;
    }
}

void test_parser_variable_declaration() {
    printf("Testing parser variable declaration...\n");
    
    char* source = "int x = 5;";
    Lexer* lexer = lexer_init(source);
    Parser* parser = parser_init(lexer);
    
    ASTNode* ast = parser_parse(parser);
    
    assert(ast != NULL);
    assert(ast->type == AST_PROGRAM);
    assert(ast->block_statement_count == 1);
    
    ASTNode* var_decl = ast->block_statements[0];
    assert(var_decl->type == AST_VAR_DECL);
    assert(var_decl->var_type == 0); // INT
    assert(strcmp(var_decl->var_name, "x") == 0);
    assert(var_decl->var_value != NULL);
    assert(var_decl->var_value->type == AST_INT);
    assert(var_decl->var_value->int_value == 5);
    
    ast_free_node(ast);
    parser_free(parser);
    lexer_free(lexer);
    
    printf("Variable declaration test passed!\n");
}

void test_parser_if_statement() {
    printf("Testing parser if statement...\n");
    
    char* source = "if (x == 5) { y = 10; } else { y = 20; }";
    Lexer* lexer = lexer_init(source);
    Parser* parser = parser_init(lexer);
    
    ASTNode* ast = parser_parse(parser);
    
    assert(ast != NULL);
    assert(ast->type == AST_PROGRAM);
    assert(ast->block_statement_count == 1);
    
    ASTNode* if_stmt = ast->block_statements[0];
    assert(if_stmt->type == AST_IF);
    assert(if_stmt->if_condition != NULL);
    assert(if_stmt->if_body != NULL);
    assert(if_stmt->else_body != NULL);
    
    // Condition: x == 5
    ASTNode* condition = if_stmt->if_condition;
    assert(condition->type == AST_BINOP);
    assert(condition->op == 4); // EQ
    assert(condition->left->type == AST_IDENTIFIER);
    assert(strcmp(condition->left->string_value, "x") == 0);
    assert(condition->right->type == AST_INT);
    assert(condition->right->int_value == 5);
    
    // Then body: y = 10;
    assert(if_stmt->if_body->type == AST_BLOCK);
    assert(if_stmt->if_body->block_statement_count == 1);
    ASTNode* then_assign = if_stmt->if_body->block_statements[0];
    assert(then_assign->type == AST_ASSIGN);
    assert(then_assign->assign_left->type == AST_IDENTIFIER);
    assert(strcmp(then_assign->assign_left->string_value, "y") == 0);
    assert(then_assign->assign_right->type == AST_INT);
    assert(then_assign->assign_right->int_value == 10);
    
    // Else body: y = 20;
    assert(if_stmt->else_body->type == AST_BLOCK);
    assert(if_stmt->else_body->block_statement_count == 1);
    ASTNode* else_assign = if_stmt->else_body->block_statements[0];
    assert(else_assign->type == AST_ASSIGN);
    assert(else_assign->assign_left->type == AST_IDENTIFIER);
    assert(strcmp(else_assign->assign_left->string_value, "y") == 0);
    assert(else_assign->assign_right->type == AST_INT);
    assert(else_assign->assign_right->int_value == 20);
    
    ast_free_node(ast);
    parser_free(parser);
    lexer_free(lexer);
    
    printf("If statement test passed!\n");
}

void test_parser_function_definition() {
    printf("Testing parser function definition...\n");
    
    char* source = "int add(int a, int b) { return a + b; }";
    Lexer* lexer = lexer_init(source);
    Parser* parser = parser_init(lexer);
    
    ASTNode* ast = parser_parse(parser);
    
    assert(ast != NULL);
    assert(ast->type == AST_PROGRAM);
    assert(ast->block_statement_count == 1);
    
    ASTNode* func_def = ast->block_statements[0];
    assert(func_def->type == AST_FUNC_DEF);
    assert(func_def->func_return_type == 0); // INT
    assert(strcmp(func_def->func_name, "add") == 0);
    assert(func_def->func_param_count == 2);
    
    // Parameters
    ASTNode* param1 = func_def->func_params[0];
    assert(param1->type == AST_VAR_DECL);
    assert(param1->var_type == 0); // INT
    assert(strcmp(param1->var_name, "a") == 0);
    
    ASTNode* param2 = func_def->func_params[1];
    assert(param2->type == AST_VAR_DECL);
    assert(param2->var_type == 0); // INT
    assert(strcmp(param2->var_name, "b") == 0);
    
    // Function body
    assert(func_def->func_body->type == AST_BLOCK);
    assert(func_def->func_body->block_statement_count == 1);
    
    ASTNode* return_stmt = func_def->func_body->block_statements[0];
    assert(return_stmt->type == AST_RETURN);
    assert(return_stmt->return_value != NULL);
    assert(return_stmt->return_value->type == AST_BINOP);
    assert(return_stmt->return_value->op == 0); // ADD
    
    ast_free_node(ast);
    parser_free(parser);
    lexer_free(lexer);
    
    printf("Function definition test passed!\n");
}

void test_parser_complete_program() {
    printf("Testing parser with a complete program...\n");
    
    char* source = 
        "#include <io>\n"
        "\n"
        "// Simple program that calculates factorial\n"
        "int factorial(int n) {\n"
        "    if (n <= 1) {\n"
        "        return 1;\n"
        "    }\n"
        "    return n * factorial(n - 1);\n"
        "}\n"
        "\n"
        "int main() {\n"
        "    int num = 5;\n"
        "    int result = factorial(num);\n"
        "    print(\"Factorial of \" + num + \" is \" + result);\n"
        "    return 0;\n"
        "}\n";
    
    Lexer* lexer = lexer_init(source);
    Parser* parser = parser_init(lexer);
    
    ASTNode* ast = parser_parse(parser);
    
    // Just verify that we get a proper AST
    assert(ast != NULL);
    assert(ast->type == AST_PROGRAM);
    assert(ast->block_statement_count > 0);
    
    // print_ast(ast, 0); // Uncomment for debugging
    
    ast_free_node(ast);
    parser_free(parser);
    lexer_free(lexer);
    
    printf("Complete program test passed!\n");
}

int main() {
    printf("Running parser tests...\n");
    
    test_parser_variable_declaration();
    test_parser_if_statement();
    test_parser_function_definition();
    test_parser_complete_program();
    
    printf("All parser tests passed!\n");
    return 0;
}