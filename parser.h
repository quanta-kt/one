#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

#include "alloc.h"
#include "ast.h"

bool parse(
    allocator_t* allocator, char* src, size_t src_len, ast_item_node** out
);

#endif  // PARSER_H
