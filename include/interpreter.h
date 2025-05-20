#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include "runtime.h"

typedef struct {
    Runtime* runtime;
} Interpreter;

Interpreter* interpreter_init(Runtime* runtime);
void interpreter_free(Interpreter* interpreter);
void* interpreter_visit(Interpreter* interpreter, ASTNode* node);
int interpreter_run(Interpreter* interpreter, ASTNode* ast);

#endif /* INTERPRETER_H */ 