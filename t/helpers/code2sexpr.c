/**
 * Parses source code passed in as argument, translates it to S-expression and
 * prints it to stdout.
 *
 * This is used in tests that assert that programs are parsed correctly and
 * yield the expected parse AST.
 */

#include <stdio.h>

/* for putting stdout to binary mode on Windows */
#ifdef _WIN32
#include <fcntl.h>
#endif

#include "arena.h"
#include "ast.h"
#include "ast_printer.h"
#include "mmio.h"
#include "mmio_alloc.h"
#include "parser.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <code>\n", argv[0]);
        return 1;
    }

#ifdef _WIN32
    _setmode(_fileno(stdout), _O_BINARY);
#endif

    arena* arena = arena_make(&mmio_alloc, mmio_get_page_size());
    allocator_t allocator = arena_get_alloc(arena);

    ast_item_node* ast = parse(&allocator, argv[1], strlen(argv[1]));
    print_ast(ast);

    arena_destroy(arena);
}

