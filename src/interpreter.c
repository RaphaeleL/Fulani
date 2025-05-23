#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "headers/interpreter.h"
#include "headers/lexer.h"
#include "headers/parser.h"

// Forward declarations
static void execute_stmt(Interpreter* interpreter, Stmt* stmt, bool* early_return, Variable* return_value);
static void print_value(Variable arg);
static void process_include(Interpreter* interpreter, const char* path);
static char* get_lib_path(const char* filename);
static char* read_file_content(const char* path);

static Environment* create_environment(Environment* enclosing) {
    Environment* env = malloc(sizeof(Environment));
    env->enclosing = enclosing;
    env->variables = NULL;
    env->variable_count = 0;
    return env;
}

static void environment_define(Environment* env, const char* name, DataType type) {
    // Check if variable already exists in current scope
    for (int i = 0; i < env->variable_count; i++) {
        if (strcmp(env->variables[i].name, name) == 0) {
            // Variable already exists, update its type
            env->variables[i].type = type;
            return;
        }
    }
    
    // Add new variable
    env->variables = realloc(env->variables, sizeof(Variable) * (env->variable_count + 1));
    env->variables[env->variable_count].name = strdup(name);
    env->variables[env->variable_count].type = type;
    env->variables[env->variable_count].is_function = false;
    env->variable_count++;
}

static Variable* environment_get(Environment* env, const char* name) {
    for (int i = 0; i < env->variable_count; i++) {
        if (strcmp(env->variables[i].name, name) == 0) {
            return &env->variables[i];
        }
    }
    
    if (env->enclosing != NULL) {
        return environment_get(env->enclosing, name);
    }
    
    return NULL;
}

static void environment_assign(Environment* env, const char* name, Variable value) {
    for (int i = 0; i < env->variable_count; i++) {
        if (strcmp(env->variables[i].name, name) == 0) {
            if (env->variables[i].type != value.type && !value.is_function) {
                fprintf(stderr, "Type mismatch in assignment to '%s'\n", name);
                return;
            }
            
            // Free old string value if necessary
            if (env->variables[i].type == TYPE_STRING && !env->variables[i].is_function) {
                free(env->variables[i].value.string_val);
            }
            
            // Copy value
            if (value.is_function) {
                env->variables[i].is_function = true;
                env->variables[i].value.function = value.value.function;
            } else {
                env->variables[i].is_function = false;
                env->variables[i].value = value.value;
                if (value.type == TYPE_STRING) {
                    env->variables[i].value.string_val = strdup(value.value.string_val);
                }
            }
            return;
        }
    }
    
    if (env->enclosing != NULL) {
        environment_assign(env->enclosing, name, value);
        return;
    }
    
    fprintf(stderr, "Undefined variable '%s'\n", name);
}

static void free_environment(Environment* env) {
    for (int i = 0; i < env->variable_count; i++) {
        free(env->variables[i].name);
        
        if (!env->variables[i].is_function) {
            if (env->variables[i].type == TYPE_STRING) {
                free(env->variables[i].value.string_val);
            } else if (env->variables[i].type == TYPE_LIST) {
                // Free all items in the list
                for (int j = 0; j < env->variables[i].value.list_val.count; j++) {
                    // For string items, we need to free the string content
                    if (env->variables[i].value.list_val.item_type == TYPE_STRING) {
                        free(env->variables[i].value.list_val.items[j]);
                    } else {
                        // For other types, we allocated memory for the value itself
                        free(env->variables[i].value.list_val.items[j]);
                    }
                }
                
                // Free the array of item pointers
                free(env->variables[i].value.list_val.items);
            }
        }
    }
    
    free(env->variables);
    free(env);
}

static Variable evaluate_expr(Interpreter* interpreter, Expr* expr) {
    Variable result = {0};
    
    switch (expr->type) {
        case EXPR_LITERAL: {
            Token* token = expr->as.literal.value;
            switch (token->type) {
                case TOKEN_INTEGER_LITERAL:
                    result.type = TYPE_INT;
                    result.value.int_val = atoi(token->lexeme);
                    break;
                case TOKEN_FLOAT_LITERAL:
                    result.type = TYPE_FLOAT;
                    result.value.float_val = atof(token->lexeme);
                    break;
                case TOKEN_STRING_LITERAL:
                    result.type = TYPE_STRING;
                    result.value.string_val = strdup(token->lexeme);
                    break;
                case TOKEN_BOOL_LITERAL:
                    result.type = TYPE_BOOL;
                    result.value.bool_val = (strcmp(token->lexeme, "true") == 0) ? 1 : 0;
                    break;
                default:
                    fprintf(stderr, "Invalid literal type: %d\n", token->type);
                    interpreter->had_error = true;
            }
            result.is_function = false;
            break;
        }
        case EXPR_BINARY: {
            // Special case for list index assignment
            if (expr->as.binary.operator.type == TOKEN_ASSIGN && 
                expr->as.binary.left->type == EXPR_LIST_ACCESS) {
                
                // Get the list variable
                VariableExpr list_var = expr->as.binary.left->as.list_access.list->as.variable;
                Variable* list_ptr = environment_get(interpreter->environment, list_var.name.lexeme);
                
                if (!list_ptr) {
                    fprintf(stderr, "Undefined variable '%s'\n", list_var.name.lexeme);
                    interpreter->had_error = true;
                    result.type = TYPE_INT; // Default type for error recovery
                    result.is_function = false;
                    result.value.int_val = 0;
                    break;
                }
                
                // Get the index
                Variable index = evaluate_expr(interpreter, expr->as.binary.left->as.list_access.index);
                // Get the value to assign
                Variable value = evaluate_expr(interpreter, expr->as.binary.right);
                
                // Check that we're working with a list
                if (list_ptr->type != TYPE_LIST) {
                    fprintf(stderr, "Cannot assign to index of non-list value\n");
                    interpreter->had_error = true;
                    break;
                }
                
                // Check that the index is an integer
                if (index.type != TYPE_INT) {
                    fprintf(stderr, "List index must be an integer\n");
                    interpreter->had_error = true;
                    break;
                }
                
                // Check that the index is in range
                int idx = index.value.int_val;
                if (idx < 0 || idx >= list_ptr->value.list_val.count) {
                    fprintf(stderr, "List index out of bounds: %d (size: %d)\n", 
                            idx, list_ptr->value.list_val.count);
                    interpreter->had_error = true;
                    break;
                }
                
                // Check that the value type matches the list item type
                if (value.type != list_ptr->value.list_val.item_type) {
                    fprintf(stderr, "Cannot assign value of type %d to list of type %d\n", 
                            value.type, list_ptr->value.list_val.item_type);
                    interpreter->had_error = true;
                    break;
                }
                
                // Free the old item (if it's a string, we need to free the memory)
                if (list_ptr->value.list_val.item_type == TYPE_STRING) {
                    free(list_ptr->value.list_val.items[idx]);
                } else {
                    free(list_ptr->value.list_val.items[idx]);
                }
                
                // Replace with the new value
                switch (value.type) {
                    case TYPE_INT: {
                        int* new_item = malloc(sizeof(int));
                        *new_item = value.value.int_val;
                        list_ptr->value.list_val.items[idx] = new_item;
                        break;
                    }
                    case TYPE_FLOAT: {
                        float* new_item = malloc(sizeof(float));
                        *new_item = value.value.float_val;
                        list_ptr->value.list_val.items[idx] = new_item;
                        break;
                    }
                    case TYPE_STRING: {
                        list_ptr->value.list_val.items[idx] = strdup(value.value.string_val);
                        break;
                    }
                    case TYPE_BOOL: {
                        int* new_item = malloc(sizeof(int));
                        *new_item = value.value.bool_val;
                        list_ptr->value.list_val.items[idx] = new_item;
                        break;
                    }
                    case TYPE_LONG: {
                        long* new_item = malloc(sizeof(long));
                        *new_item = value.value.long_val;
                        list_ptr->value.list_val.items[idx] = new_item;
                        break;
                    }
                    case TYPE_DOUBLE: {
                        double* new_item = malloc(sizeof(double));
                        *new_item = value.value.double_val;
                        list_ptr->value.list_val.items[idx] = new_item;
                        break;
                    }
                    default:
                        fprintf(stderr, "Unsupported type for list assignment\n");
                        interpreter->had_error = true;
                        break;
                }
                
                // Return the assigned value
                result = value;
                
                break;
            }
            
            // Regular binary expression
            Variable left = evaluate_expr(interpreter, expr->as.binary.left);
            Variable right = evaluate_expr(interpreter, expr->as.binary.right);
            
            result.is_function = false;
            
            // Special handling for string concatenation
            if (expr->as.binary.operator.type == TOKEN_PLUS && left.type == TYPE_STRING && right.type == TYPE_STRING) {
                result.type = TYPE_STRING;
                // Allocate enough space for both strings plus null terminator
                size_t len1 = strlen(left.value.string_val);
                size_t len2 = strlen(right.value.string_val);
                result.value.string_val = malloc(len1 + len2 + 1);
                
                // Concatenate the strings
                strcpy(result.value.string_val, left.value.string_val);
                strcat(result.value.string_val, right.value.string_val);
                
                // Free the original strings
                free(left.value.string_val);
                free(right.value.string_val);
                break;
            }
            
            // Regular numeric operations
            if (left.type != right.type) {
                fprintf(stderr, "Operands must be of the same type\n");
                interpreter->had_error = true;
                break;
            }
            
            result.type = TYPE_INT;  // All comparison operators return int (boolean)
            
            switch (expr->as.binary.operator.type) {
                case TOKEN_PLUS:
                    result.type = left.type;
                    if (left.type == TYPE_INT)
                        result.value.int_val = left.value.int_val + right.value.int_val;
                    else if (left.type == TYPE_FLOAT)
                        result.value.float_val = left.value.float_val + right.value.float_val;
                    else if (left.type == TYPE_LONG)
                        result.value.int_val = left.value.int_val + right.value.int_val; // Using int_val for longs too
                    else if (left.type == TYPE_DOUBLE)
                        result.value.float_val = left.value.float_val + right.value.float_val; // Using float_val for doubles too
                    break;
                case TOKEN_MINUS:
                    result.type = left.type;
                    if (left.type == TYPE_INT)
                        result.value.int_val = left.value.int_val - right.value.int_val;
                    else if (left.type == TYPE_FLOAT)
                        result.value.float_val = left.value.float_val - right.value.float_val;
                    break;
                case TOKEN_MULTIPLY:
                    result.type = left.type;
                    if (left.type == TYPE_INT)
                        result.value.int_val = left.value.int_val * right.value.int_val;
                    else if (left.type == TYPE_FLOAT)
                        result.value.float_val = left.value.float_val * right.value.float_val;
                    break;
                case TOKEN_DIVIDE:
                    result.type = left.type;
                    if (left.type == TYPE_INT) {
                        if (right.value.int_val == 0) {
                            fprintf(stderr, "Division by zero\n");
                            interpreter->had_error = true;
                            break;
                        }
                        result.value.int_val = left.value.int_val / right.value.int_val;
                    } else if (left.type == TYPE_FLOAT) {
                        if (right.value.float_val == 0.0) {
                            fprintf(stderr, "Division by zero\n");
                            interpreter->had_error = true;
                            break;
                        }
                        result.value.float_val = left.value.float_val / right.value.float_val;
                    }
                    break;
                case TOKEN_MODULO:
                    result.type = left.type;
                    if (left.type == TYPE_INT) {
                        if (right.value.int_val == 0) {
                            fprintf(stderr, "Modulo by zero\n");
                            interpreter->had_error = true;
                            break;
                        }
                        result.value.int_val = left.value.int_val % right.value.int_val;
                    } else if (left.type == TYPE_FLOAT) {
                        fprintf(stderr, "Modulo operation not supported for float values\n");
                        interpreter->had_error = true;
                    }
                    break;
                case TOKEN_EQUALS:
                    if (left.type == TYPE_INT)
                        result.value.int_val = left.value.int_val == right.value.int_val;
                    else if (left.type == TYPE_FLOAT)
                        result.value.int_val = left.value.float_val == right.value.float_val;
                    break;
                case TOKEN_NOT_EQUALS:
                    if (left.type == TYPE_INT)
                        result.value.int_val = left.value.int_val != right.value.int_val;
                    else if (left.type == TYPE_FLOAT)
                        result.value.int_val = left.value.float_val != right.value.float_val;
                    break;
                case TOKEN_LESS:
                    if (left.type == TYPE_INT)
                        result.value.int_val = left.value.int_val < right.value.int_val;
                    else if (left.type == TYPE_FLOAT)
                        result.value.int_val = left.value.float_val < right.value.float_val;
                    break;
                case TOKEN_LESS_EQUAL:
                    if (left.type == TYPE_INT)
                        result.value.int_val = left.value.int_val <= right.value.int_val;
                    else if (left.type == TYPE_FLOAT)
                        result.value.int_val = left.value.float_val <= right.value.float_val;
                    break;
                case TOKEN_GREATER:
                    if (left.type == TYPE_INT)
                        result.value.int_val = left.value.int_val > right.value.int_val;
                    else if (left.type == TYPE_FLOAT)
                        result.value.int_val = left.value.float_val > right.value.float_val;
                    break;
                case TOKEN_GREATER_EQUAL:
                    if (left.type == TYPE_INT)
                        result.value.int_val = left.value.int_val >= right.value.int_val;
                    else if (left.type == TYPE_FLOAT)
                        result.value.int_val = left.value.float_val >= right.value.float_val;
                    break;
                default:
                    fprintf(stderr, "Invalid binary operator\n");
                    interpreter->had_error = true;
            }
            break;
        }
        case EXPR_UNARY: {
            Variable operand = evaluate_expr(interpreter, expr->as.unary.operand);
            result.type = operand.type;
            result.is_function = false;
            
            switch (expr->as.unary.operator.type) {
                case TOKEN_MINUS:
                    if (operand.type == TYPE_INT)
                        result.value.int_val = -operand.value.int_val;
                    else if (operand.type == TYPE_FLOAT)
                        result.value.float_val = -operand.value.float_val;
                    break;
                default:
                    fprintf(stderr, "Invalid unary operator\n");
                    interpreter->had_error = true;
            }
            break;
        }
        case EXPR_VARIABLE: {
            Variable* var = environment_get(interpreter->environment, expr->as.variable.name.lexeme);
            if (var == NULL) {
                fprintf(stderr, "Undefined variable '%s'\n", expr->as.variable.name.lexeme);
                interpreter->had_error = true;
                // Initialize with default value for error recovery
                result.type = TYPE_INT;
                result.is_function = false;
                result.value.int_val = 0;
                break;
            }
            
            // Make a copy of the variable to return
            result.type = var->type;
            result.is_function = var->is_function;
            
            if (var->is_function) {
                result.value.function = var->value.function;
            } else {
                if (var->type == TYPE_STRING) {
                    result.value.string_val = strdup(var->value.string_val);
                } else {
                    result.value = var->value;
                }
            }
            break;
        }
        case EXPR_ASSIGN: {
            Variable value = evaluate_expr(interpreter, expr->as.assign.value);
            environment_assign(interpreter->environment, expr->as.assign.name.lexeme, value);
            result = value;
            break;
        }
        case EXPR_CALL: {
            Variable callee = evaluate_expr(interpreter, expr->as.call.callee);
            
            if (strcmp(expr->as.call.callee->as.variable.name.lexeme, "println") == 0) {
                // Handle built-in println function
                for (int i = 0; i < expr->as.call.arg_count; i++) {
                    Variable arg = evaluate_expr(interpreter, expr->as.call.arguments[i]);
                    print_value(arg);
                    if (i < expr->as.call.arg_count - 1) {
                        printf(" ");
                    }
                }
                printf("\n");
                result.type = TYPE_VOID;
                result.is_function = false;
            } else if (strcmp(expr->as.call.callee->as.variable.name.lexeme, "print") == 0) {
                // Handle built-in print function (like println but without newline)
                for (int i = 0; i < expr->as.call.arg_count; i++) {
                    Variable arg = evaluate_expr(interpreter, expr->as.call.arguments[i]);
                    print_value(arg);
                    if (i < expr->as.call.arg_count - 1) {
                        printf(" ");
                    }
                }
                // No newline for print function
                result.type = TYPE_VOID;
                result.is_function = false;
            } else if (callee.is_function) {
                FunctionStmt* func = callee.value.function.declaration;
                Environment* previous = interpreter->environment;
                
                // Create new environment for function with closure as parent
                interpreter->environment = create_environment(callee.value.function.closure);
                
                // Evaluate all arguments in the caller's environment
                Variable* args = malloc(sizeof(Variable) * expr->as.call.arg_count);
                for (int i = 0; i < expr->as.call.arg_count; i++) {
                    // Switch back to the caller's environment to evaluate arguments
                    Environment* arg_env = interpreter->environment;
                    interpreter->environment = previous;
                    args[i] = evaluate_expr(interpreter, expr->as.call.arguments[i]);
                    interpreter->environment = arg_env;
                }
                
                // Now set up parameters in the function's environment
                for (int i = 0; i < expr->as.call.arg_count; i++) {
                    // Define the parameter in the function environment
                    environment_define(interpreter->environment, func->params[i].lexeme, func->param_types[i]);
                    
                    // Create parameter variable
                    Variable param = {0};
                    param.name = strdup(func->params[i].lexeme);
                    param.type = func->param_types[i];
                    param.is_function = false;
                    
                    // Copy argument value to parameter
                    if (args[i].type == TYPE_STRING) {
                        param.value.string_val = strdup(args[i].value.string_val);
                        free(args[i].value.string_val); // Free the original
                    } else {
                        param.value = args[i].value;
                    }
                    
                    // Assign the parameter in the function environment
                    environment_assign(interpreter->environment, param.name, param);
                }
                free(args);
                
                // Use a dedicated return value
                Variable return_value = {0};
                bool early_return = false;
                
                // Execute function body with early return flag and return value
                execute_stmt(interpreter, func->body, &early_return, &return_value);
                
                if (early_return) {
                    // If we got an early return, use the provided return value
                    result = return_value;
                } else {
                    // Otherwise use default value for the return type
                    result.type = func->return_type;
                    result.is_function = false;
                    if (func->return_type == TYPE_INT) {
                        result.value.int_val = 0;
                    } else if (func->return_type == TYPE_FLOAT) {
                        result.value.float_val = 0.0;
                    } else if (func->return_type == TYPE_STRING) {
                        result.value.string_val = strdup("");
                    } else if (func->return_type == TYPE_BOOL) {
                        result.value.bool_val = 0; // Default to false
                    } else if (func->return_type == TYPE_LONG) {
                        result.value.long_val = 0L;
                    } else if (func->return_type == TYPE_DOUBLE) {
                        result.value.double_val = 0.0;
                    } else if (func->return_type == TYPE_LIST) {
                        result.value.list_val.items = NULL;
                        result.value.list_val.count = 0;
                    }
                }
                
                // Restore environment
                interpreter->environment = previous;
            } else {
                fprintf(stderr, "Can only call functions\n");
                interpreter->had_error = true;
            }
            break;
        }
        case EXPR_LIST_ACCESS: {
            // Get the list variable
            VariableExpr list_var = expr->as.list_access.list->as.variable;
            Variable* list_ptr = environment_get(interpreter->environment, list_var.name.lexeme);
            
            if (!list_ptr) {
                fprintf(stderr, "Undefined variable '%s'\n", list_var.name.lexeme);
                interpreter->had_error = true;
                result.type = TYPE_INT; // Default type for error recovery
                result.is_function = false;
                result.value.int_val = 0;
                break;
            }
            
            if (list_ptr->type != TYPE_LIST) {
                fprintf(stderr, "Cannot access index on a non-list value\n");
                interpreter->had_error = true;
                result.type = TYPE_INT; // Default type for error recovery
                result.is_function = false;
                result.value.int_val = 0;
                break;
            }
            
            Variable index = evaluate_expr(interpreter, expr->as.list_access.index);
            
            if (index.type != TYPE_INT) {
                fprintf(stderr, "List index must be an integer\n");
                interpreter->had_error = true;
                result.type = TYPE_INT; // Default type for error recovery
                result.is_function = false;
                result.value.int_val = 0;
                break;
            }
            
            int idx = index.value.int_val;
            if (idx < 0 || idx >= list_ptr->value.list_val.count) {
                fprintf(stderr, "List index out of bounds: %d (size: %d)\n", 
                        idx, list_ptr->value.list_val.count);
                interpreter->had_error = true;
                result.type = TYPE_INT; // Default type for error recovery
                result.is_function = false;
                result.value.int_val = 0;
                break;
            }
            
            // Get the value at the specified index
            void* item = list_ptr->value.list_val.items[idx];
            result.type = list_ptr->value.list_val.item_type;
            result.is_function = false;
            
            // Copy the value based on its type
            switch (result.type) {
                case TYPE_INT:
                    result.value.int_val = *((int*)item);
                    break;
                case TYPE_FLOAT:
                    result.value.float_val = *((float*)item);
                    break;
                case TYPE_STRING:
                    result.value.string_val = strdup((char*)item);
                    break;
                case TYPE_BOOL:
                    result.value.bool_val = *((int*)item);
                    break;
                case TYPE_LONG:
                    result.value.long_val = *((long*)item);
                    break;
                case TYPE_DOUBLE:
                    result.value.double_val = *((double*)item);
                    break;
                default:
                    fprintf(stderr, "Unsupported list item type\n");
                    interpreter->had_error = true;
                    break;
            }
            break;
        }
        case EXPR_LIST_METHOD: {
            // Get the list variable
            VariableExpr list_var = expr->as.list_method.list->as.variable;
            Variable* list_ptr = environment_get(interpreter->environment, list_var.name.lexeme);
            
            if (!list_ptr) {
                fprintf(stderr, "Undefined variable '%s'\n", list_var.name.lexeme);
                interpreter->had_error = true;
                result.type = TYPE_VOID;
                result.is_function = false;
                break;
            }
            
            if (list_ptr->type != TYPE_LIST) {
                fprintf(stderr, "Cannot call method on a non-list value\n");
                interpreter->had_error = true;
                result.type = TYPE_VOID;
                result.is_function = false;
                break;
            }
            
            if (expr->as.list_method.method == TOKEN_ADD) {
                // Evaluate the argument to add
                Variable item = evaluate_expr(interpreter, expr->as.list_method.argument);
                
                // If this is the first item, set the item type
                if (list_ptr->value.list_val.count == 0) {
                    list_ptr->value.list_val.item_type = item.type;
                }
                
                // Check that the new item matches the existing list type
                if (item.type != list_ptr->value.list_val.item_type) {
                    fprintf(stderr, "Cannot add item of type %d to list of type %d\n", 
                            item.type, list_ptr->value.list_val.item_type);
                    interpreter->had_error = true;
                    result.type = TYPE_VOID;
                    result.is_function = false;
                    break;
                }
                
                // Allocate memory for the new item based on its type
                void* new_item = NULL;
                switch (item.type) {
                    case TYPE_INT: {
                        int* int_item = malloc(sizeof(int));
                        *int_item = item.value.int_val;
                        new_item = int_item;
                        break;
                    }
                    case TYPE_FLOAT: {
                        float* float_item = malloc(sizeof(float));
                        *float_item = item.value.float_val;
                        new_item = float_item;
                        break;
                    }
                    case TYPE_STRING: {
                        new_item = strdup(item.value.string_val);
                        break;
                    }
                    case TYPE_BOOL: {
                        int* bool_item = malloc(sizeof(int));
                        *bool_item = item.value.bool_val;
                        new_item = bool_item;
                        break;
                    }
                    case TYPE_LONG: {
                        long* long_item = malloc(sizeof(long));
                        *long_item = item.value.long_val;
                        new_item = long_item;
                        break;
                    }
                    case TYPE_DOUBLE: {
                        double* double_item = malloc(sizeof(double));
                        *double_item = item.value.double_val;
                        new_item = double_item;
                        break;
                    }
                    default:
                        fprintf(stderr, "Unsupported item type for list.add\n");
                        interpreter->had_error = true;
                        break;
                }
                
                // Add the item to the list
                list_ptr->value.list_val.items = realloc(list_ptr->value.list_val.items, 
                                                    sizeof(void*) * (list_ptr->value.list_val.count + 1));
                list_ptr->value.list_val.items[list_ptr->value.list_val.count] = new_item;
                list_ptr->value.list_val.count++;
                
                // Return void (the add method doesn't return a value)
                result.type = TYPE_VOID;
                result.is_function = false;
            }
            else if (expr->as.list_method.method == TOKEN_REMOVE) {
                // Evaluate the index to remove
                Variable index = evaluate_expr(interpreter, expr->as.list_method.argument);
                
                if (index.type != TYPE_INT) {
                    fprintf(stderr, "List index must be an integer\n");
                    interpreter->had_error = true;
                    result.type = TYPE_VOID;
                    result.is_function = false;
                    break;
                }
                
                int idx = index.value.int_val;
                if (idx < 0 || idx >= list_ptr->value.list_val.count) {
                    fprintf(stderr, "List index out of bounds: %d (size: %d)\n", 
                            idx, list_ptr->value.list_val.count);
                    interpreter->had_error = true;
                    result.type = TYPE_VOID;
                    result.is_function = false;
                    break;
                }
                
                // Free the memory for the item being removed
                if (list_ptr->value.list_val.item_type == TYPE_STRING) {
                    free(list_ptr->value.list_val.items[idx]);
                } else {
                    free(list_ptr->value.list_val.items[idx]);
                }
                
                // Shift all remaining elements down
                for (int i = idx; i < list_ptr->value.list_val.count - 1; i++) {
                    list_ptr->value.list_val.items[i] = list_ptr->value.list_val.items[i + 1];
                }
                
                // Shrink the list size
                list_ptr->value.list_val.count--;
                
                // Return void (the remove method doesn't return a value)
                result.type = TYPE_VOID;
                result.is_function = false;
            }
            break;
        }
        case EXPR_LIST_PROPERTY: {
            // Get the list variable
            VariableExpr list_var = expr->as.list_property.list->as.variable;
            Variable* list_ptr = environment_get(interpreter->environment, list_var.name.lexeme);
            
            if (!list_ptr) {
                fprintf(stderr, "Undefined variable '%s'\n", list_var.name.lexeme);
                interpreter->had_error = true;
                result.type = TYPE_INT; // Default type for error recovery
                result.is_function = false;
                result.value.int_val = 0;
                break;
            }
            
            if (list_ptr->type != TYPE_LIST) {
                fprintf(stderr, "Cannot access property on a non-list value\n");
                interpreter->had_error = true;
                result.type = TYPE_INT; // Default type for error recovery
                result.is_function = false;
                result.value.int_val = 0;
                break;
            }
            
            if (expr->as.list_property.property == TOKEN_LENGTH) {
                // Return the length of the list
                result.type = TYPE_INT;
                result.is_function = false;
                result.value.int_val = list_ptr->value.list_val.count;
            }
            break;
        }
    }
    
    return result;
}

static void print_value(Variable arg) {
    switch (arg.type) {
        case TYPE_INT:
            printf("%d", arg.value.int_val);
            break;
        case TYPE_FLOAT:
            printf("%f", arg.value.float_val);
            break;
        case TYPE_STRING:
            printf("%s", arg.value.string_val);
            break;
        case TYPE_BOOL:
            printf("%s", arg.value.bool_val ? "true" : "false");
            break;
        case TYPE_LONG:
            printf("%ld", arg.value.long_val);
            break;
        case TYPE_DOUBLE:
            printf("%lf", arg.value.double_val);
            break;
        case TYPE_LIST:
            printf("[");
            for (int j = 0; j < arg.value.list_val.count; j++) {
                void* item = arg.value.list_val.items[j];
                
                // Print the item based on its type
                switch (arg.value.list_val.item_type) {
                    case TYPE_INT:
                        printf("%d", *((int*)item));
                        break;
                    case TYPE_FLOAT:
                        printf("%f", *((float*)item));
                        break;
                    case TYPE_STRING:
                        printf("\"%s\"", (char*)item);
                        break;
                    case TYPE_BOOL:
                        printf("%s", (*((int*)item)) ? "true" : "false");
                        break;
                    case TYPE_LONG:
                        printf("%ld", *((long*)item));
                        break;
                    case TYPE_DOUBLE:
                        printf("%lf", *((double*)item));
                        break;
                    default:
                        printf("?");
                        break;
                }
                
                if (j < arg.value.list_val.count - 1) {
                    printf(", ");
                }
            }
            printf("]");
            break;
        default:
            break;
    }
}

static void execute_stmt(Interpreter* interpreter, Stmt* stmt, bool* early_return, Variable* return_value) {
    if (*early_return) return;  // Skip execution if we've already returned
    
    switch (stmt->type) {
        case STMT_EXPRESSION:
            evaluate_expr(interpreter, stmt->as.expression);
            break;
        case STMT_VAR_DECL: {
            Variable var = {0};
            var.name = strdup(stmt->as.var_decl.name.lexeme);
            var.type = stmt->as.var_decl.type;
            var.is_function = false;
            
            environment_define(interpreter->environment, var.name, var.type);
            
            if (stmt->as.var_decl.initializer != NULL) {
                Variable init = evaluate_expr(interpreter, stmt->as.var_decl.initializer);
                
                // Special case: implicit int->bool conversion for boolean variables
                if (var.type == TYPE_BOOL && init.type == TYPE_INT) {
                    var.value.bool_val = init.value.int_val ? 1 : 0;
                }
                // Special case: int->long conversion
                else if (var.type == TYPE_LONG && init.type == TYPE_INT) {
                    var.value.long_val = (long)init.value.int_val;
                }
                // Special case: float->double conversion
                else if (var.type == TYPE_DOUBLE && init.type == TYPE_FLOAT) {
                    var.value.double_val = (double)init.value.float_val;
                }
                // Regular case: types match
                else if (init.type == var.type) {
                    if (var.type == TYPE_STRING) {
                        var.value.string_val = strdup(init.value.string_val);
                        free(init.value.string_val);
                    } else {
                        var.value = init.value;
                    }
                } else {
                    fprintf(stderr, "Type mismatch in variable initialization\n");
                    interpreter->had_error = true;
                    return;
                }
            } else {
                // Initialize with default values
                switch (var.type) {
                    case TYPE_INT:
                        var.value.int_val = 0;
                        break;
                    case TYPE_FLOAT:
                        var.value.float_val = 0.0;
                        break;
                    case TYPE_STRING:
                        var.value.string_val = strdup("");
                        break;
                    case TYPE_BOOL:
                        var.value.bool_val = 0; // false
                        break;
                    case TYPE_LONG:
                        var.value.long_val = 0L;
                        break;
                    case TYPE_DOUBLE:
                        var.value.double_val = 0.0;
                        break;
                    case TYPE_LIST:
                        var.value.list_val.items = NULL;
                        var.value.list_val.count = 0;
                        break;
                    default:
                        break;
                }
            }
            
            environment_assign(interpreter->environment, var.name, var);
            break;
        }
        case STMT_BLOCK: {
            // Only create a new environment if this is not a variable declaration block
            Environment* previous = interpreter->environment;
            if (stmt->as.block.count == 0 || stmt->as.block.statements[0]->type != STMT_VAR_DECL) {
                interpreter->environment = create_environment(previous);
            }
            
            for (int i = 0; i < stmt->as.block.count; i++) {
                execute_stmt(interpreter, stmt->as.block.statements[i], early_return, return_value);
                if (*early_return) break;  // Exit the block early if we've returned
            }
            
            if (stmt->as.block.count == 0 || stmt->as.block.statements[0]->type != STMT_VAR_DECL) {
                interpreter->environment = previous;
            }
            break;
        }
        case STMT_IF: {
            Variable condition = evaluate_expr(interpreter, stmt->as.if_stmt.condition);
            if (condition.type != TYPE_INT && condition.type != TYPE_BOOL) {
                fprintf(stderr, "Condition must be an integer or boolean\n");
                interpreter->had_error = true;
                return;
            }
            
            bool is_true = (condition.type == TYPE_BOOL) 
                           ? condition.value.bool_val 
                           : condition.value.int_val != 0;
            
            if (is_true) {
                execute_stmt(interpreter, stmt->as.if_stmt.then_branch, early_return, return_value);
            } else if (stmt->as.if_stmt.else_branch != NULL) {
                execute_stmt(interpreter, stmt->as.if_stmt.else_branch, early_return, return_value);
            }
            break;
        }
        case STMT_WHILE: {
            for (;;) {
                Variable condition = evaluate_expr(interpreter, stmt->as.while_stmt.condition);
                if (condition.type != TYPE_INT && condition.type != TYPE_BOOL) {
                    fprintf(stderr, "Condition must be an integer or boolean\n");
                    interpreter->had_error = true;
                    return;
                }
                
                bool is_true = (condition.type == TYPE_BOOL) 
                               ? condition.value.bool_val 
                               : condition.value.int_val != 0;
                if (!is_true) break;
                
                execute_stmt(interpreter, stmt->as.while_stmt.body, early_return, return_value);
                if (*early_return) break;  // Exit the loop early if we've returned
            }
            break;
        }
        case STMT_FOR: {
            // Create a new environment for the for loop (for variable scope)
            Environment* previous = interpreter->environment;
            interpreter->environment = create_environment(previous);
            
            // Execute the initialization once
            if (stmt->as.for_stmt.init != NULL) {
                execute_stmt(interpreter, stmt->as.for_stmt.init, early_return, return_value);
                if (*early_return) {
                    interpreter->environment = previous;
                    return;
                }
            }
            
            // Execute the condition-body-increment loop
            while (true) {
                // Check condition (if any)
                if (stmt->as.for_stmt.condition != NULL) {
                    Variable condition = evaluate_expr(interpreter, stmt->as.for_stmt.condition);
                    if (condition.type != TYPE_INT && condition.type != TYPE_BOOL) {
                        fprintf(stderr, "For loop condition must be an integer or boolean\n");
                        interpreter->had_error = true;
                        interpreter->environment = previous;
                        return;
                    }
                    
                    // Exit if condition is false
                    bool is_true = (condition.type == TYPE_BOOL) 
                                   ? condition.value.bool_val 
                                   : condition.value.int_val != 0;
                    if (!is_true) break;
                }
                
                // Execute body
                execute_stmt(interpreter, stmt->as.for_stmt.body, early_return, return_value);
                if (*early_return) {
                    interpreter->environment = previous;
                    return;
                }
                
                // Execute increment (if any)
                if (stmt->as.for_stmt.increment != NULL) {
                    evaluate_expr(interpreter, stmt->as.for_stmt.increment);
                }
            }
            
            // Restore the previous environment
            interpreter->environment = previous;
            break;
        }
        case STMT_FUNCTION: {
            // Store the function in the environment
            Variable func = {0};
            func.name = strdup(stmt->as.function.name.lexeme);
            func.type = stmt->as.function.return_type;
            func.is_function = true;
            
            // Deep copy the function declaration
            func.value.function.declaration = malloc(sizeof(FunctionStmt));
            func.value.function.declaration->name = stmt->as.function.name;
            func.value.function.declaration->return_type = stmt->as.function.return_type;
            func.value.function.declaration->param_count = stmt->as.function.param_count;
            
            // Copy parameters
            func.value.function.declaration->params = malloc(sizeof(Token) * stmt->as.function.param_count);
            func.value.function.declaration->param_types = malloc(sizeof(DataType) * stmt->as.function.param_count);
            for (int i = 0; i < stmt->as.function.param_count; i++) {
                func.value.function.declaration->params[i] = stmt->as.function.params[i];
                func.value.function.declaration->param_types[i] = stmt->as.function.param_types[i];
            }
            
            // Copy body
            func.value.function.declaration->body = stmt->as.function.body;
            func.value.function.closure = interpreter->environment;
            
            environment_define(interpreter->environment, func.name, func.type);
            environment_assign(interpreter->environment, func.name, func);
            
            // If this is the main function, execute it immediately
            if (strcmp(func.name, "main") == 0) {
                Variable main_return = {0};
                bool main_early_return = false;
                execute_stmt(interpreter, stmt->as.function.body, &main_early_return, &main_return);
            }
            break;
        }
        case STMT_RETURN: {
            if (stmt->as.return_stmt.expression != NULL) {
                Variable value = evaluate_expr(interpreter, stmt->as.return_stmt.expression);
                
                // Copy the return value
                if (value.type == TYPE_STRING) {
                    return_value->type = value.type;
                    return_value->is_function = false;
                    return_value->value.string_val = strdup(value.value.string_val);
                } else {
                    *return_value = value;
                }
                
                *early_return = true;  // Set early return flag
            }
            break;
        }
        case STMT_INCLUDE: {
            // Get the include path
            const char* path_str = stmt->as.include.path.lexeme;
            
            // Fix path - remove quotes
            char* path = strdup(path_str);
            if (path[0] == '"') {
                // Remove opening quote
                memmove(path, path + 1, strlen(path));
            }
            
            // Remove closing quote if exists
            int len = strlen(path);
            if (len > 0 && path[len - 1] == '"') {
                path[len - 1] = '\0';
            }
            
            // Process the include
            process_include(interpreter, path);
            
            // Free temporary path
            free(path);
            break;
        }
    }
}

void interpreter_init(Interpreter* interpreter) {
    interpreter->globals = create_environment(NULL);
    interpreter->environment = interpreter->globals;
    interpreter->had_error = false;

    // Add built-in println function
    Variable println = {0};
    println.name = strdup("println");
    println.type = TYPE_VOID;
    println.is_function = true;
    environment_define(interpreter->globals, println.name, println.type);
    environment_assign(interpreter->globals, println.name, println);
    
    // Add built-in print function (no newline)
    Variable print = {0};
    print.name = strdup("print");
    print.type = TYPE_VOID;
    print.is_function = true;
    environment_define(interpreter->globals, print.name, print.type);
    environment_assign(interpreter->globals, print.name, print);
}

void interpreter_interpret(Interpreter* interpreter, Stmt** statements, int count) {
    for (int i = 0; i < count; i++) {
        Variable return_value = {0};
        bool early_return = false;
        execute_stmt(interpreter, statements[i], &early_return, &return_value);
        if (interpreter->had_error) break;
    }
}

void interpreter_cleanup(Interpreter* interpreter) {
    free_environment(interpreter->globals);
}

// Process an include statement by loading and interpreting the included file
static void process_include(Interpreter* interpreter, const char* path) {
    // Get the full path to the library file
    char* full_path = get_lib_path(path);
    
    if (full_path == NULL) {
        fprintf(stderr, "Error: Could not find library file: %s\n", path);
        interpreter->had_error = true;
        return;
    }
    
    // Read the file content
    char* source = read_file_content(full_path);
    if (source == NULL) {
        fprintf(stderr, "Error: Could not read library file: %s\n", full_path);
        free(full_path);
        interpreter->had_error = true;
        return;
    }
    
    printf("Including file: %s\n", full_path);
    
    // Parse the included file
    Lexer lexer;
    lexer_init(&lexer, source);
    
    Parser parser;
    parser_init(&parser, &lexer);
    
    int count = 0;
    Stmt** statements = parse(&parser, &count);
    
    if (parser.had_error) {
        fprintf(stderr, "Error: Failed to parse included file: %s\n", full_path);
        interpreter->had_error = true;
        free(source);
        free(full_path);
        return;
    }
    
    // Save the current environment
    Environment* previous = interpreter->environment;
    
    // Execute each statement in the included file
    for (int i = 0; i < count; i++) {
        Variable return_value = {0};
        bool early_return = false;
        execute_stmt(interpreter, statements[i], &early_return, &return_value);
        
        if (interpreter->had_error) {
            fprintf(stderr, "Error: Failed to execute statement in included file: %s\n", full_path);
            break;
        }
    }
    
    // Free resources
    for (int i = 0; i < count; i++) {
        free_stmt(statements[i]);
    }
    free(statements);
    free(source);
    free(full_path);
}

// Get the full path to a library file
static char* get_lib_path(const char* filename) {
    // Debug: Print the filename we're looking for
    printf("Looking for file: '%s'\n", filename);
    
    // Check if it's a relative path or stdlib reference
    if (filename[0] == '/' || 
        (filename[0] == '.' && filename[1] == '/') || 
        (filename[0] == '.' && filename[1] == '.' && filename[2] == '/')) {
        // It's a relative or absolute path, use it directly
        return strdup(filename);
    }
    
    // Try looking in the standard library
    char* stdlib_path = malloc(strlen("lib/stdlib/") + strlen(filename) + 1);
    sprintf(stdlib_path, "lib/stdlib/%s", filename);
    
    // Check if the file exists
    FILE* file = fopen(stdlib_path, "r");
    if (file) {
        fclose(file);
        printf("Found in stdlib: %s\n", stdlib_path);
        return stdlib_path;
    }
    
    // File not found in stdlib, free the path
    free(stdlib_path);
    
    // Try looking in the current directory
    FILE* local_file = fopen(filename, "r");
    if (local_file) {
        fclose(local_file);
        printf("Found in current directory: %s\n", filename);
        return strdup(filename);
    }
    
    // File not found
    printf("File not found: %s\n", filename);
    return NULL;
}

// Read the contents of a file
static char* read_file_content(const char* path) {
    FILE* file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file: %s\n", path);
        return NULL;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    
    // Allocate buffer
    char* buffer = malloc(size + 1);
    if (!buffer) {
        fprintf(stderr, "Error: Memory allocation failed when reading file: %s\n", path);
        fclose(file);
        return NULL;
    }
    
    // Read file content
    size_t bytes_read = fread(buffer, 1, size, file);
    if (bytes_read < (size_t)size) {
        fprintf(stderr, "Error: Failed to read entire file: %s (read %zu of %ld bytes)\n", 
                path, bytes_read, size);
        free(buffer);
        fclose(file);
        return NULL;
    }
    
    buffer[bytes_read] = '\0';
    
    printf("Successfully read %zu bytes from file: %s\n", bytes_read, path);
    
    fclose(file);
    return buffer;
}
