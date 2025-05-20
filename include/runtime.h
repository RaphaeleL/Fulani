#ifndef RUNTIME_H
#define RUNTIME_H

#include <stdlib.h>
#include "ast.h"

// Forward declarations
typedef struct Environment Environment;
typedef struct Value Value;
typedef struct Runtime Runtime;

// Type enumeration
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_CHAR,
    TYPE_DOUBLE,
    TYPE_LONG,
    TYPE_STRING,
    TYPE_LIST,
    TYPE_STRUCT,
    TYPE_VOID,
    TYPE_FUNCTION,
    TYPE_NULL
} ValueType;

// Value structure
struct Value {
    ValueType type;
    union {
        int int_value;
        float float_value;
        char char_value;
        double double_value;
        long long_value;
        char* string_value;
        
        struct {
            Value** list_elements;
            int list_size;
        };
        
        struct {
            char* struct_name;
            char** field_names;
            Value** field_values;
            int field_count;
        };
        
        struct {
            ASTNode* func_def;
            Environment* closure;
        };
    };
};

// Environment structure (symbol table)
struct Environment {
    char** names;
    Value** values;
    int size;
    int capacity;
    Environment* parent;
};

// Runtime structure
struct Runtime {
    Environment* global_env;
    Environment* current_env;
    Value* last_result;
    int debug_mode;
};

// Function prototypes
Runtime* runtime_init(int debug_mode);
void runtime_free(Runtime* runtime);
Environment* environment_init(Environment* parent);
void environment_free(Environment* env);
void environment_define(Environment* env, const char* name, Value* value);
Value* environment_get(Environment* env, const char* name);
void environment_set(Environment* env, const char* name, Value* value);
Value* value_create(ValueType type);
void value_free(Value* value);

#endif /* RUNTIME_H */ 