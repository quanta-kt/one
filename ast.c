#include "ast.h"

#include <stdlib.h>

#include "alloc.h"

ast_node* make_ast_num(allocator_t* allocator, double long value) {
    ast_node* node = ALLOC(allocator, ast_node);
    node->type = AST_NUM;
    node->num.value = value;

    return node;
}

ast_node* make_ast_bool(allocator_t* allocator, bool value) {
    ast_node* node = ALLOC(allocator, ast_node);
    node->type = AST_BOOL;
    node->boolean.value = value;

    return node;
}

ast_node* make_ast_str(
    allocator_t* allocator, char* str, size_t len, size_t size
) {
    ast_node* node = ALLOC(allocator, ast_node);
    node->type = AST_STR;
    node->str.str = str;
    node->str.len = len;
    node->str.size = size;

    return node;
}

ast_node* make_ast_identifier(allocator_t* allocator, char* start, size_t len) {
    ast_node* node = ALLOC(allocator, ast_node);
    node->type = AST_IDEN;
    node->identifier.start = start;
    node->identifier.len = len;

    return node;
}

ast_node* make_ast_binary(
    allocator_t* allocator, token_type op, ast_node* left, ast_node* right
) {
    ast_node* node = ALLOC(allocator, ast_node);
    node->type = AST_BINARY;
    node->binary.op = op;
    node->binary.left = left;
    node->binary.right = right;

    return node;
}

void free_ast(allocator_t* allocator, ast_node* node) {
    switch (node->type) {
        case AST_STR:
            FREE_ARRAY(allocator, node->str.str, char, node->str.size);
            break;

        case AST_BINARY: {
            free_ast(allocator, node->binary.left);
            free_ast(allocator, node->binary.right);
            break;
        }
    }

    FREE(allocator, node, ast_node);
}
