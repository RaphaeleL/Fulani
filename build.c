#define SHL_IMPLEMENTATION
#define SHL_STRIP_PREFIX
#include "./build.h"

Cmd cmd = {0};

int main() {
    auto_rebuild("build.c");

    push(&cmd, "gcc");
    push(&cmd, "src/fulani.c", "src/lexer.c", "src/parser.c", "src/ast.c", "src/interpreter.c");
    push(&cmd, "-Wall", "-Wextra", "-std=c11");
    push(&cmd, "-o", "fulani");
    if (!run_always(&cmd)) return 1;

    return 0;
}
