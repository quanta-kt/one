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

        if (memcmp(arg, "--", sizeof("--")) == 0 || arg[0] == '-') {

            /* Flags must appear after path */
            if (ret.path == NULL) {
                fprintf(stderr, "Expected filename before flag '%s'\n", arg);
                print_usage_and_die(exec);
            }

            fprintf(stderr, "Invalid flag: '%s'\n", arg);
            print_usage_and_die(exec);
        } else {
            ret.path = arg;
        }
    }

    if (ret.path == NULL) {
        fprintf(stderr, "Expected path to a source file.\n");
        print_usage_and_die(exec);
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

    ret = compile_file(&args, in);
    fclose(in);

    return ret;
}
