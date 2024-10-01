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

ast_expr_node* make_ast_unary(
    allocator_t* allocator, token_type op, ast_expr_node* expr
) {
    ast_expr_node* node = ALLOC(allocator, ast_expr_node);
    node->type = AST_UNARY;
    node->unary.op = op;
    node->unary.expr = expr;

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

        case AST_UNARY: {
            free_ast_expr(allocator, node->unary.expr);
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

ast_stmt_node* make_ast_var_decl(
    allocator_t* allocator, token name, ast_expr_node* value, bool mut
) {
    ast_stmt_node* node = ALLOC(allocator, ast_stmt_node);
    *node = stmt_node_defaults;

    node->type = AST_VAR_DECL;
    node->var_decl.name = name;
    node->var_decl.value = value;
    node->var_decl.mut = mut;

    return node;
}

ast_stmt_node* make_ast_block(allocator_t* allocator, ast_stmt_node* body) {
    ast_stmt_node* node = ALLOC(allocator, ast_stmt_node);
    *node = stmt_node_defaults;

    node->type = AST_BLOCK;
    node->block.body = body;
}

ast_stmt_node* make_ast_if_else(
    allocator_t* allocator,
    ast_expr_node* condition,
    ast_stmt_node* body,
    ast_stmt_node* else_body
) {
    ast_stmt_node* node = ALLOC(allocator, ast_stmt_node);
    *node = stmt_node_defaults;

    node->type = AST_IF_ELSE;
    node->if_else = (ast_node_if_else){
        .condition = condition,
        .body = body,
        .else_body = else_body,
    };

    return node;
}

static void _free_ast_stmt(allocator_t* allocator, ast_stmt_node* node) {
    switch (node->type) {
        case AST_EXPR_STMT: {
            free_ast_expr(allocator, node->expr_stmt.expr);
            break;
        }

        case AST_VAR_DECL: {
            free_ast_expr(allocator, node->var_decl.value);
            break;
        }

        case AST_BLOCK: {
            free_ast_stmt(allocator, node->block.body);
            break;
        }

        case AST_IF_ELSE: {
            free_ast_expr(allocator, node->if_else.condition);
            free_ast_stmt(allocator, node->if_else.body);
            free_ast_stmt(allocator, node->if_else.else_body);
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
