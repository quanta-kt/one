#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ast.h"
#include "ast_printer.h"
#include "parser.h"

size_t read_all(FILE* in, char** ptr) {
    const size_t block_size = 255;

    char* buf = NULL;

    // characters allocated
    size_t size = 0;

    // characters read
    size_t len = 0;

    size_t last_read = 0;
    do {
        size += block_size;
        buf = realloc(buf, sizeof(char) * size);

        char* block_start = buf + (size - block_size);
        len += (last_read = fread(block_start, sizeof(char), block_size, in));

    } while (last_read == block_size);

    buf[len] = '\0';

    *ptr = buf;
    return len;
}

int run_repl() {
    char* line;
    size_t len;

    allocator_t* allocator = gpa();

    while (getline(&line, &len, stdin) != -1) {
        ast_stmt_node* ast = parse(allocator, line, len);

        print_ast(ast);
        free_ast_stmt(allocator, ast);
    }

    return 0;
}

int run_file(FILE* file) {
    char* buf;
    size_t len = read_all(file, &buf);

    allocator_t* allocator = gpa();

    ast_stmt_node* ast = parse(allocator, buf, len);
    print_ast(ast);

    free_ast_stmt(allocator, ast);
    free(buf);
    return 0;
}

FILE* open_file_or_die(char* path) {
    FILE* f = fopen(path, "r");

    if (f == NULL) {
        fprintf(stderr, "Unable to open file '%s'\n", path);
        exit(1);
    }

    return f;
}

void print_usage_and_die(char* program) {
    printf("usage: %s [filename]\n", program);
    exit(1);
}

int main(int argc, char** argv) {
    FILE* in = NULL;

    switch (argc) {
        case 0:
        case 1:
            in = stdin;
            break;

        case 2:
            in = open_file_or_die(argv[1]);
            break;

        default:
            print_usage_and_die(*argv);
    }

    if (in == stdin && isatty(STDIN_FILENO)) {
        return run_repl();
    }

    int ret = run_file(in);
    if (in != stdin) {
        fclose(in);
    }

    return ret;
}
