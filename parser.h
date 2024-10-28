#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

#include "alloc.h"
#include "ast.h"

ast_item_node* parse(allocator_t* allocator, char* src, size_t src_len);

#endif  // PARSER_H
