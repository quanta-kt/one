/**
 * Parses source code passed in as argument, translates it to S-expression and
 * prints it to stdout.
 *
 * This is used in tests that assert that programs are parsed correctly and
 * yield the expected parse AST.
 */

#include <stdio.h>

#include "alloc.h"
#include "ast.h"
#include "ast_printer.h"
#include "parser.h"

int main(int argc, char** argv) {
    int ret = 0;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <code>\n", argv[0]);
        return 1;
    }

    allocator_t* allocator = gpa();

    ast_item_node* ast;

    if (parse(allocator, argv[1], strlen(argv[1]), &ast)) {
        print_ast(ast);
    } else {
        ret = 1;
    }

    free_ast(allocator, ast);

    return ret;
}
