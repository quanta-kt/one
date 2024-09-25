#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "ast_printer.h"
#include "parser.h"

int main() {
    char* line;
    size_t len;

    allocator_t* allocator = gpa();

    while (getline(&line, &len, stdin) != -1) {
        ast_expr_node* ast = parse(allocator, line, len);

        print_ast(ast);
        free_ast_expr(allocator, ast);
    }
}
