#include "ast.h"

#include <stdlib.h>

#include "alloc.h"

ast_typename* make_ast_typename(
    allocator_t* allocator, ast_typename_type type
) {
    ast_typename* typename = ALLOC(allocator, ast_typename);
    typename->type = type;
    return typename;
}

ast_typename* make_ast_typename_function(
    allocator_t* allocator, vec_typename params, ast_typename* return_type
) {
    ast_typename* ret = make_ast_typename(allocator, TYPE_NAME_FUNCTION);
    ret->as.function.params = params;
    ret->as.function.return_type = return_type;
    return ret;
}

void free_ast_typename(allocator_t* allocator, ast_typename* node) {
    switch (node->type) {
        case TYPE_NAME_BOOLEAN:
        case TYPE_NAME_STRING:
        case TYPE_NAME_NUMBER:
            break;

        case TYPE_NAME_FUNCTION: {
            ast_typename** param;
            vec_foreach(&node->as.function.params, param) {
                free_ast_typename(allocator, *param);
            }
            vec_free(&node->as.function.params);

            free_ast_typename(allocator, node->as.function.return_type);
        }; break;
    }

    FREE(allocator, node, ast_typename);
}

static void free_params(allocator_t* allocator, ast_param* params);

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

ast_expr_node* make_ast_call(
    allocator_t* allocator, ast_expr_node* function, vec_expr args
) {
    ast_expr_node* node = ALLOC(allocator, ast_expr_node);
    node->type = AST_CALL;
    node->call = (ast_node_call){
        .function = function,
        .args = args,
    };

    return node;
}

ast_expr_node* make_ast_lambda(
    allocator_t* allocator, ast_param* params, ast_stmt_node* body
) {
    ast_expr_node* node = ALLOC(allocator, ast_expr_node);
    node->type = AST_LAMBDA;
    node->lambda = (ast_node_lambda){
        .body = body,
        .params = params,
    };

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

        case AST_CALL: {
            vec_free(&node->call.args);
            free_ast_expr(allocator, node->call.function);
            break;
        }

        case AST_LAMBDA: {
            free_ast_stmt(allocator, node->lambda.body);
            free_params(allocator, node->lambda.params);
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
    allocator_t* allocator,
    token name,
    ast_typename* typename,
    ast_expr_node* value,
    bool mut
) {
    ast_stmt_node* node = ALLOC(allocator, ast_stmt_node);
    *node = stmt_node_defaults;

    node->type = AST_VAR_DECL;
    node->var_decl.name = name;
    node->var_decl.typename = typename;
    node->var_decl.value = value;
    node->var_decl.mut = mut;

    return node;
}

ast_stmt_node* make_ast_block(allocator_t* allocator, ast_stmt_node* body) {
    ast_stmt_node* node = ALLOC(allocator, ast_stmt_node);
    *node = stmt_node_defaults;

    node->type = AST_BLOCK;
    node->block.body = body;

    return node;
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

ast_stmt_node* make_ast_while(
    allocator_t* allocator, ast_expr_node* condition, ast_stmt_node* body
) {
    ast_stmt_node* node = ALLOC(allocator, ast_stmt_node);
    *node = stmt_node_defaults;

    node->type = AST_WHILE;
    node->while_ = (ast_node_while){
        .condition = condition,
        .body = body,
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
            if (node->var_decl.value != NULL)
                free_ast_expr(allocator, node->var_decl.value);
            if (node->var_decl.typename != NULL)
                free_ast_typename(allocator, node->var_decl.typename);
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

        case AST_WHILE: {
            free_ast_expr(allocator, node->while_.condition);
            free_ast_stmt(allocator, node->while_.body);
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

ast_param* make_ast_param(allocator_t* allocator, token name, ast_typename* type) {
    ast_param* p = ALLOC(allocator, ast_param);
    p->name = name;
    p->type = type;
    p->next = NULL;

    return p;
}

ast_item_node* make_ast_function(
    allocator_t* allocator, token name, ast_param* params, ast_stmt_node* body
) {
    ast_item_node* node = ALLOC(allocator, ast_item_node);
    node->type = AST_FN;
    node->function.name = name;
    node->function.params = params;
    node->function.body = body;

    return node;
}

static void free_params(allocator_t* allocator, ast_param* params) {
    ast_param* curr = params;
    while (curr != NULL) {
        ast_param* t = curr;
        curr = t->next;

        free_ast_typename(allocator, t->type);
        FREE(allocator, t, ast_param);
    }
}

void free_ast_item(allocator_t* allocator, ast_item_node* node) {
    switch (node->type) {
        case AST_FN: {
            ast_param* curr = node->function.params;
            if (curr != NULL) {
                free_params(allocator, node->function.params);
            }

            free_ast_stmt(allocator, node->function.body);
        } break;

        default:
            break;
    }

    FREE(allocator, node, ast_item_node);
}
