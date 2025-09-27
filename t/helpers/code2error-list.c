/**
 * Parses code passed into it's arugment and prints out line separated error
 * codes of all the errors that occur.
 *
 * This is used in tests that make assertions about errors.
 */

#include <stdio.h>

#include "alloc.h"
#include "ast.h"
#include "error_printer.h"
#include "parser.h"

void print_error_code(const char* source, span_info* span,
                      const error_desc_t error, va_list args) {
    printf("E%04d\n", error.code);
}

error_printer_t error_printer = (error_printer_t){
    .error = &print_error_code,
};

int main(int argc, char** argv) {
    int ret = 0;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <code>\n", argv[0]);
        return 1;
    }

    allocator_t* allocator = gpa();

    ast_item_node* ast;

    if (!parse_with_error_printer(allocator, argv[1], strlen(argv[1]),
                                 error_printer, &ast)) {
        ret = 1;
    }

    free_ast(allocator, ast);

    return ret;
}
