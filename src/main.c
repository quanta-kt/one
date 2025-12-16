#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ast.h"
#include "parser.h"
#include "typecheck.h"

struct compiler_args {
    char* path;
};

const struct compiler_args DEFAULT_ARGS = (struct compiler_args){
    .path = NULL,
};

void print_usage_and_die(char* program) {
    fprintf(stderr, "Usage: %s [path]\n", program);
    exit(1);
}

struct compiler_args parse_args(int argc, char** argv) {
    struct compiler_args ret = DEFAULT_ARGS;
    char* exec = *(argv++);

    argc--;

    while (argc--) {
        char* arg = *(argv++);

        if (ret.path != NULL) {
            print_usage_and_die(exec);
        }

        if (memcmp(arg, "--", sizeof("--")) == 0 || arg[0] == '-') {
            fprintf(stderr, "Invalid flag: '%s'\n", arg);
            print_usage_and_die(exec);
        } else {
            ret.path = arg;
        }
    }

    return ret;
}

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

int run_repl(struct compiler_args* args) {
    (void) args;

    char* line = NULL;
    size_t len;

    allocator_t* allocator = gpa();

    while (getline(&line, &len, stdin) != -1) {
        ast_item_node* ast = parse(allocator, line, len);

        if (typecheck(allocator, ast)) {
            fprintf(stderr, "NOT IMPLEMENTED: Code execution is WIP.\n");
            exit(1);
        }

        free_ast(allocator, ast);
    }

    return 0;
}

int compile_file(struct compiler_args* args, FILE* file) {
    (void) args;

    int ret = 0;

    char* buf;
    size_t len = read_all(file, &buf);

    allocator_t* allocator = gpa();

    ast_item_node* ast = parse(allocator, buf, len);

    if (!typecheck(allocator, ast)) {
        ret = 1;
        goto cleanup;
    }

    fprintf(stderr, "NOT IMPLEMENTED: Code execution is WIP.\n");
    ret = 2;

cleanup:
    free_ast(allocator, ast);
    free(buf);

    return ret;
}

FILE* open_file_or_die(char* path) {
    FILE* f = fopen(path, "r");

    if (f == NULL) {
        fprintf(stderr, "Unable to open file '%s'\n", path);
        exit(1);
    }

    return f;
}

int main(int argc, char** argv) {
    int ret;

    struct compiler_args args = parse_args(argc, argv);
    FILE* in = args.path == NULL ? stdin : open_file_or_die(args.path);

    if (in == stdin && isatty(STDIN_FILENO)) {
        ret = run_repl(&args);
    } else {
        ret = compile_file(&args, in);
    }

    if (in != stdin) {
        fclose(in);
    }

    return ret;
}
