#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "ast_walk_printer.h"
#include "parser.h"

int main() {
    char* line;
    size_t len;

    allocator_t* allocator = gpa();

    while (getline(&line, &len, stdin) != -1) {
        ast_node* ast = parse(allocator, line, len);

        ast_walk(&walk_printer, ast);

        free_ast(allocator, ast);

        printf("\n");
    }
}
