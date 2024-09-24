#ifndef AST_H
#define AST_H

#include "alloc.h"
#include "lex.h"

typedef enum {
    AST_NUM,
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
        ast_node_str str;
        ast_node_binary binary;
        ast_node_identifier identifier;
    };

    ast_node_type type;
} ast_node;

struct _ast_walker;

typedef struct _ast_walker {
    void (*walk_binary)(struct _ast_walker*, ast_node_binary*);
    void (*walk_num)(struct _ast_walker*, ast_node_num*);
    void (*walk_iden)(struct _ast_walker*, ast_node_identifier*);
    void (*walk_str)(struct _ast_walker*, ast_node_str*);
} ast_walker;

ast_node* make_ast_num(allocator_t* allocator, double long value);
ast_node* make_ast_str(allocator_t* allocator, char* str, size_t len, size_t size);
ast_node* make_ast_identifier(allocator_t* allocator, char* start, size_t len);
ast_node* make_ast_binary(
    allocator_t* allocator, token_type op, ast_node* left, ast_node* right
);

void free_ast(allocator_t* allocator, ast_node* node);

void ast_walk(ast_walker* walker, ast_node* node);

#endif  // AST_H
