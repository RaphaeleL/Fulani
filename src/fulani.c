#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

static char* read_file(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }
    
    fseek(file, 0L, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);
    
    char* buffer = (char*)malloc(file_size + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }
    
    size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
    if (bytes_read < file_size) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }
    
    buffer[bytes_read] = '\0';
    
    fclose(file);
    return buffer;
}

static void run_file(const char* path, bool debug) {
    char* source = read_file(path);
    
    Lexer lexer;
    lexer_init(&lexer, source);
    
    Parser parser;
    parser_init(&parser, &lexer);
    
    int count;
    Stmt** statements = parse(&parser, &count);
    
    if (parser.had_error) {
        free(source);
        // Free statements
        for (int i = 0; i < count; i++) {
            free_stmt(statements[i]);
        }
        free(statements);
        exit(65);
    }
    
    // Print AST if debug mode is enabled
    if (debug) {
        print_ast(statements, count);
    }
    
    Interpreter interpreter;
    interpreter_init(&interpreter);
    interpreter.debug = debug;  // Set debug flag in interpreter
    
    interpreter_interpret(&interpreter, statements, count);
    
    if (interpreter.had_error) {
        free(source);
        // Free statements
        for (int i = 0; i < count; i++) {
            free_stmt(statements[i]);
        }
        free(statements);
        interpreter_cleanup(&interpreter);
        exit(70);
    }
    
    // Cleanup
    free(source);
    for (int i = 0; i < count; i++) {
        free_stmt(statements[i]);
    }
    free(statements);
    interpreter_cleanup(&interpreter);
}

int main(int argc, const char* argv[]) {
    bool debug = false;
    const char* script_path = NULL;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0) {
            debug = true;
        } else if (script_path == NULL) {
            script_path = argv[i];
        } else {
            fprintf(stderr, "Usage: ownlang [--debug] script\n");
            exit(64);
        }
    }
    
    if (script_path == NULL) {
        fprintf(stderr, "Usage: ownlang [--debug] script\n");
        exit(64);
    }
    
    run_file(script_path, debug);
    return 0;
}
