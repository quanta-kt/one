/**
 * Typechecks source code passed in as arguemnt.
 * Exits with 0 if the typecheck passes or 1 if it fails.
 * Any other status code should be interpreted as an error.
 */

#include <stdio.h>

#include "alloc.h"
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

    allocator_t* allocator = gpa();

    ast_item_node* ast = parse(allocator, code, len);
    if (typecheck(allocator, ast)) {
        ret = 0;
    }

    free_ast(allocator, ast);

    return ret;
}

