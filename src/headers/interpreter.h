#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <stdbool.h>
#include "ast.h"

// Forward declarations
struct Environment;
typedef struct Environment Environment;

typedef struct {
    char* name;
    DataType type;
    union {
        int int_val;
        float float_val;
        char* string_val;
        struct {
            FunctionStmt* declaration;
            Environment* closure;
        } function;
    } value;
    bool is_function;
} Variable;

struct Environment {
    Environment* enclosing;
    Variable* variables;
    int variable_count;
};

typedef struct {
    Environment* globals;
    Environment* environment;
    bool had_error;
    bool debug;  // Debug flag to enable AST printing
} Interpreter;

void interpreter_init(Interpreter* interpreter);
void interpreter_interpret(Interpreter* interpreter, Stmt** statements, int count);
void interpreter_cleanup(Interpreter* interpreter);

#endif // INTERPRETER_H 