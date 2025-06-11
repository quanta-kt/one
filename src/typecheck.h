#ifndef TYPECHECK_H
#define TYPECHECK_H

#include "ast.h"

/**
 * Walks down the AST to typecheck.
 * Returns true if the types are sound, false otherwise.
 */
bool typecheck(allocator_t* allocator, ast_item_node* ast);

#endif  // TYPECHECK_H
