#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* for putting stdout to binary mode on Windows */
#ifdef _WIN32
#include <fcntl.h>
#endif

#include "arena.h"
#include "ast.h"
#include "mmio.h"
#include "mmio_alloc.h"
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

int compile_file(struct compiler_args* args, mmio_mapping* mapping) {
    (void) args;

    int ret = 0;

    arena* arena = arena_make(&mmio_alloc, mmio_get_page_size());
    allocator_t allocator = arena_get_alloc(arena);

    ast_item_node* ast = parse(&allocator, (char*) mapping->ptr, mapping->length);

    if (!typecheck(&allocator, ast)) {
        ret = 1;
        goto cleanup;
    }

    fprintf(stderr, "NOT IMPLEMENTED: Code execution is WIP.\n");

cleanup:
    arena_destroy(arena);

    return ret;
}

mmio_mapping open_file_or_die(char* path) {
    mmio_mapping ret = { 0 };
    if (!mmio_mm_path(path, &ret)) {
        fprintf(stderr, "Unable to open file '%s'\n", path);
        exit(1);
    }

    return ret;
}

int main(int argc, char** argv) {
    int ret;

#ifdef _WIN32
    _setmode(_fileno(stdout), _O_BINARY);
#endif

    struct compiler_args args = parse_args(argc, argv);

    mmio_mapping mapping = open_file_or_die(args.path);
    ret = compile_file(&args, &mapping);
    mmio_unmap(&mapping);

    return ret;
}
