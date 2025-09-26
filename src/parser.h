#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

#include "alloc.h"
#include "ast.h"
#include "error_printer.h"

/**
 * Parses the source code and builds AST.
 * The resulting AST nodes are placed into \p{out}.
 *
 * Even when parsing fails due to syntax errors at some locations, parse()
 * *will* return and set the result into \p{out}. The nodes where the
 * syntax error occurred will however be "synthetic", i.e. crated by
 * the parser to satisfy completeness of the AST and not actually parsed
 * from source.
 *
 * NOTE: Not all codepaths currently build the "error nodes" and simply
 *       exit() when an error occurs. The migration is in progress.
 *
 * See: `erorr_printer.{c,h}` for information on error printing.
 *
 * @param allocator Allocator to use for allocating AST nodes.
 * @param src Source to parse. Does not have to be NULL-terminated.
 * @param src_len Length of \p{src}.
 * @param error_printer Error printer to use to report errors in source code.
 * @param out The pointer where resultant AST nodes are placed.
 *
 * @return true if parsing succeeded without any errors, false otherwise.
 */
bool parse_with_error_printer(
    allocator_t* allocator, char* src, size_t src_len,
    error_printer_t error_printer,
    ast_item_node** out);

/**
 * Parses the source code and builds AST.
 * The resulting AST nodes are placed into \p{out}.
 *
 * This simply calls `parse_with_error_printer` with the default
 * error_printer which prints the errors to stderr.
 *
 * See: `parse_with_error_printer` for more information.
 * See: `erorr_printer.{c,h}` for information on error printing.
 *
 * @param allocator Allocator to use for allocating AST nodes.
 * @param src Source to parse. Does not have to be NULL-terminated.
 * @param src_len Length of \p{src}.
 * @param out The pointer where resultant AST nodes are placed.
 *
 * @return true if parsing succeeded without any errors, false otherwise.
 */
bool parse(
    allocator_t* allocator, char* src, size_t src_len, ast_item_node** out
);

#endif  // PARSER_H
