#include "ast.h"

#include <stdlib.h>

#include "alloc.h"

ast_expr_node* make_ast_num(allocator_t* allocator, double long value) {
    ast_expr_node* node = ALLOC(allocator, ast_expr_node);
    node->type = AST_NUM;
    node->num.value = value;

    return node;
}

ast_expr_node* make_ast_bool(allocator_t* allocator, bool value) {
    ast_expr_node* node = ALLOC(allocator, ast_expr_node);
    node->type = AST_BOOL;
    node->boolean.value = value;

    return node;
}

ast_expr_node* make_ast_str(
    allocator_t* allocator, char* str, size_t len, size_t size
) {
    ast_expr_node* node = ALLOC(allocator, ast_expr_node);
    node->type = AST_STR;
    node->str.str = str;
    node->str.len = len;
    node->str.size = size;

    return node;
}

ast_expr_node* make_ast_identifier(
    allocator_t* allocator, char* start, size_t len
) {
    ast_expr_node* node = ALLOC(allocator, ast_expr_node);
    node->type = AST_IDEN;
    node->identifier.start = start;
    node->identifier.len = len;

    return node;
}

ast_expr_node* make_ast_binary(
    allocator_t* allocator,
    token_type op,
    ast_expr_node* left,
    ast_expr_node* right
) {
    ast_expr_node* node = ALLOC(allocator, ast_expr_node);
    node->type = AST_BINARY;
    node->binary.op = op;
    node->binary.left = left;
    node->binary.right = right;

    return node;
}

void free_ast_expr(allocator_t* allocator, ast_expr_node* node) {
    switch (node->type) {
        case AST_STR:
            FREE_ARRAY(allocator, node->str.str, char, node->str.size);
            break;

        case AST_BINARY: {
            free_ast_expr(allocator, node->binary.left);
            free_ast_expr(allocator, node->binary.right);
            break;
        }
    }

    FREE(allocator, node, ast_expr_node);
}

ast_stmt_node stmt_node_defaults = {
    .next = NULL,
};

ast_stmt_node* make_ast_expr_stmt(allocator_t* allocator, ast_expr_node* expr) {
    ast_stmt_node* node = ALLOC(allocator, ast_stmt_node);
    *node = stmt_node_defaults;

    node->type = AST_EXPR_STMT;
    node->expr_stmt.expr = expr;

    return node;
}

static void _free_ast_stmt(allocator_t* allocator, ast_stmt_node* node) {
    switch (node->type) {
        case AST_EXPR_STMT: {
            free_ast_expr(allocator, node->expr_stmt.expr);
            break;
        }
    }

    FREE(allocator, node, ast_stmt_node);
}

void free_ast_stmt(allocator_t* allocator, ast_stmt_node* node) {
    while (node != NULL) {
        ast_stmt_node* next = node->next;
        _free_ast_stmt(allocator, node);
        node = next;
    }
}
