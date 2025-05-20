#ifndef LANG_H
#define LANG_H

#define LANG_VERSION "0.1.0"
#define LANG_NAME "MyLang"

// CLI flags
typedef struct {
    char* input_file;
    char* output_file;
    int compile_mode;
    int debug_mode;
} LangOptions;

LangOptions parse_arguments(int argc, char** argv);
void print_usage();
void print_version();

#endif /* LANG_H */ 