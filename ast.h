#ifndef AST_H
#define AST_H

#include "alloc.h"
#include "lex.h"

typedef enum {
    AST_NUM,
    AST_BOOL,
    AST_STR,
    AST_IDEN,
    AST_BINARY,
} ast_expr_node_type;

struct _ast_expr_node;

typedef struct {
    long double value;
} ast_node_num;

typedef struct {
    char* str;

    // length of the string
    size_t len;

    // size of the buffer allocated at 'str'
    size_t size;
} ast_node_str;

typedef struct {
    bool value;
} ast_node_bool;

typedef struct {
    char* start;
    size_t len;
} ast_node_identifier;

typedef struct {
    token_type op;
    struct _ast_expr_node* left;
    struct _ast_expr_node* right;
} ast_node_binary;

typedef struct _ast_expr_node {
    union {
        ast_node_num num;
        ast_node_bool boolean;
        ast_node_str str;
        ast_node_binary binary;
        ast_node_identifier identifier;
    };

    ast_expr_node_type type;
} ast_expr_node;

ast_expr_node* make_ast_num(allocator_t* allocator, double long value);
ast_expr_node* make_ast_bool(allocator_t* allocator, bool value);
ast_expr_node* make_ast_str(
    allocator_t* allocator, char* str, size_t len, size_t size
);
ast_expr_node* make_ast_identifier(
    allocator_t* allocator, char* start, size_t len
);
ast_expr_node* make_ast_binary(
    allocator_t* allocator,
    token_type op,
    ast_expr_node* left,
    ast_expr_node* right
);

void free_ast_expr(allocator_t* allocator, ast_expr_node* node);

#define AST_EXPR_WALKER(name, return_type)                               \
    struct _##name;                                                      \
    typedef struct _##name {                                             \
        return_type (*walk_binary)(struct _##name*, ast_node_binary*);   \
        return_type (*walk_num)(struct _##name*, ast_node_num*);         \
        return_type (*walk_iden)(struct _##name*, ast_node_identifier*); \
        return_type (*walk_str)(struct _##name*, ast_node_str*);         \
        return_type (*walk_bool)(struct _##name*, ast_node_bool*);       \
    } name;                                                              \
                                                                         \
    return_type name##_walk(name* walker, ast_expr_node* node) {         \
        switch (node->type) {                                            \
            case AST_NUM:                                                \
                return walker->walk_num(walker, &node->num);             \
                                                                         \
            case AST_BOOL:                                               \
                return walker->walk_bool(walker, &node->boolean);        \
                                                                         \
            case AST_IDEN:                                               \
                return walker->walk_iden(walker, &node->identifier);     \
                                                                         \
            case AST_BINARY:                                             \
                return walker->walk_binary(walker, &node->binary);       \
                                                                         \
            case AST_STR:                                                \
                return walker->walk_str(walker, &node->str);             \
        }                                                                \
    }

typedef enum {
    AST_EXPR_STMT,
    AST_VAR_DECL,
    AST_BLOCK,
    AST_IF_ELSE,
} ast_stmt_node_type;

struct _ast_stmt_node;

typedef struct {
    ast_expr_node* expr;
} ast_node_expr_stmt;

typedef struct {
    token name;
    ast_expr_node* value;
    bool mut;
} ast_node_var_decl;

typedef struct {
    struct _ast_stmt_node* body;
} ast_node_block;

typedef struct {
    ast_expr_node* condition;
    struct _ast_stmt_node* body;
    struct _ast_stmt_node* else_body;
} ast_node_if_else;

typedef struct _ast_stmt_node {
    union {
        ast_node_expr_stmt expr_stmt;
        ast_node_var_decl var_decl;
        ast_node_block block;
        ast_node_if_else if_else;
    };
    ast_stmt_node_type type;

    struct _ast_stmt_node* next;
} ast_stmt_node;

ast_stmt_node* make_ast_expr_stmt(allocator_t* allocator, ast_expr_node* expr);
ast_stmt_node* make_ast_var_decl(
    allocator_t* allocator, token name, ast_expr_node* value, bool mut
);
ast_stmt_node* make_ast_block(allocator_t* allocator, ast_stmt_node* body);

ast_stmt_node* make_ast_if_else(
    allocator_t* allocator,
    ast_expr_node* condition,
    ast_stmt_node* body,
    ast_stmt_node* else_body
);

void free_ast_stmt(allocator_t* allocator, ast_stmt_node* node);

#define AST_STMT_WALKER(name, return_type)                                   \
    struct _##name;                                                          \
    typedef struct _##name {                                                 \
        return_type (*walk_expr_stmt)(struct _##name*, ast_node_expr_stmt*); \
        return_type (*walk_var_decl)(struct _##name*, ast_node_var_decl*);   \
        return_type (*walk_block)(struct _##name*, ast_node_block*);         \
        return_type (*walk_if_else)(struct _##name*, ast_node_if_else*);     \
    } name;                                                                  \
                                                                             \
    return_type name##_walk(name* walker, ast_stmt_node* node) {             \
        switch (node->type) {                                                \
            case AST_EXPR_STMT: {                                            \
                return walker->walk_expr_stmt(walker, &node->expr_stmt);     \
            }                                                                \
            case AST_VAR_DECL: {                                             \
                return walker->walk_var_decl(walker, &node->var_decl);       \
            }                                                                \
            case AST_BLOCK: {                                                \
                return walker->walk_block(walker, &node->block);             \
            }                                                                \
            case AST_IF_ELSE: {                                              \
                return walker->walk_if_else(walker, &node->if_else);         \
            }                                                                \
        }                                                                    \
    }

#endif  // AST_H
