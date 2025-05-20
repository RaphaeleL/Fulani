#ifndef BUILTINS_H
#define BUILTINS_H

#include "runtime.h"

void register_builtins(Runtime* runtime);
Value* builtin_print(Runtime* runtime, Value** args, int arg_count);
Value* builtin_len(Runtime* runtime, Value** args, int arg_count);
Value* builtin_input(Runtime* runtime, Value** args, int arg_count);

#endif /* BUILTINS_H */ 