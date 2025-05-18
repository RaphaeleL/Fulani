#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interpreter.h"

// Forward declarations
static void execute_stmt(Interpreter* interpreter, Stmt* stmt, bool* early_return, Variable* return_value);

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
        if (env->variables[i].type == TYPE_STRING) {
            free(env->variables[i].value.string_val);
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
                default:
                    fprintf(stderr, "Invalid literal type: %d\n", token->type);
                    interpreter->had_error = true;
            }
            result.is_function = false;
            break;
        }
        case EXPR_BINARY: {
            Variable left = evaluate_expr(interpreter, expr->as.binary.left);
            Variable right = evaluate_expr(interpreter, expr->as.binary.right);
            
            if (left.type != right.type) {
                fprintf(stderr, "Operands must be of the same type\n");
                interpreter->had_error = true;
                break;
            }
            
            result.type = TYPE_INT;  // All comparison operators return int (boolean)
            result.is_function = false;
            
            switch (expr->as.binary.operator.type) {
                case TOKEN_PLUS:
                    result.type = left.type;
                    if (left.type == TYPE_INT)
                        result.value.int_val = left.value.int_val + right.value.int_val;
                    else if (left.type == TYPE_FLOAT)
                        result.value.float_val = left.value.float_val + right.value.float_val;
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
                        default:
                            break;
                    }
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
                        default:
                            break;
                    }
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
    }
    
    return result;
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
                if (init.type != var.type) {
                    fprintf(stderr, "Type mismatch in variable initialization\n");
                    interpreter->had_error = true;
                    return;
                }
                var.value = init.value;
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
            if (condition.type != TYPE_INT) {
                fprintf(stderr, "Condition must be an integer\n");
                interpreter->had_error = true;
                return;
            }
            
            if (condition.value.int_val) {
                execute_stmt(interpreter, stmt->as.if_stmt.then_branch, early_return, return_value);
            } else if (stmt->as.if_stmt.else_branch != NULL) {
                execute_stmt(interpreter, stmt->as.if_stmt.else_branch, early_return, return_value);
            }
            break;
        }
        case STMT_WHILE: {
            for (;;) {
                Variable condition = evaluate_expr(interpreter, stmt->as.while_stmt.condition);
                if (condition.type != TYPE_INT) {
                    fprintf(stderr, "Condition must be an integer\n");
                    interpreter->had_error = true;
                    return;
                }
                
                if (!condition.value.int_val) break;
                
                execute_stmt(interpreter, stmt->as.while_stmt.body, early_return, return_value);
                if (*early_return) break;  // Exit the loop early if we've returned
            }
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