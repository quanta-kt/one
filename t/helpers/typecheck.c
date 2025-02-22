/**
 * Typechecks source code passed in as arguemnt.
 * Exits with 0 if the typecheck passes or 1 if it fails.
 * Any other status code should be interpreted as an error.
 */

#include <stdio.h>

#include "arena.h"
#include "mmio.h"
#include "mmio_alloc.h"
#include "ast.h"
#include "parser.h"
#include "typecheck.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <code>\n", argv[0]);
        return 2;
    }

    char* code = argv[1];
    size_t len = strlen(code);
    int ret = 1;

    arena* arena = arena_make(&mmio_alloc, mmio_get_page_size());
    allocator_t allocator = arena_get_alloc(arena);

    ast_item_node* ast;

    if (parse(&allocator, code, len, &ast) && typecheck(&allocator, ast)) {
        ret = 0;
    }

    arena_destroy(arena);

    return ret;
}

