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
} ast_node_type;

struct _ast_node;

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
    struct _ast_node* left;
    struct _ast_node* right;
} ast_node_binary;

typedef struct _ast_node {
    union {
        ast_node_num num;
        ast_node_bool boolean;
        ast_node_str str;
        ast_node_binary binary;
        ast_node_identifier identifier;
    };

    ast_node_type type;
} ast_node;

ast_node* make_ast_num(allocator_t* allocator, double long value);
ast_node* make_ast_bool(allocator_t* allocator, bool value);
ast_node* make_ast_str(
    allocator_t* allocator, char* str, size_t len, size_t size
);
ast_node* make_ast_identifier(allocator_t* allocator, char* start, size_t len);
ast_node* make_ast_binary(
    allocator_t* allocator, token_type op, ast_node* left, ast_node* right
);

void free_ast(allocator_t* allocator, ast_node* node);

#define AST_WALKER(name, return_type)                                    \
    struct _##name;                                                      \
    typedef struct _##name {                                             \
        return_type (*walk_binary)(struct _##name*, ast_node_binary*);   \
        return_type (*walk_num)(struct _##name*, ast_node_num*);         \
        return_type (*walk_iden)(struct _##name*, ast_node_identifier*); \
        return_type (*walk_str)(struct _##name*, ast_node_str*);         \
        return_type (*walk_bool)(struct _##name*, ast_node_bool*);       \
    } name;                                                              \
                                                                         \
    return_type name##_walk(name* walker, ast_node* node) {              \
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

#endif  // AST_H
