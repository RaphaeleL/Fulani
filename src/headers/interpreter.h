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
        int bool_val;       // Boolean value (0 for false, 1 for true)
        long long_val;      // Long integer value
        double double_val;  // Double precision value
        struct {
            void** items;    // List items
            int count;       // List size
            DataType item_type; // Type of items in the list
        } list_val;
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