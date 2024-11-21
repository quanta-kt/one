#ifndef AST_H
#define AST_H

#include "alloc.h"
#include "lex.h"
#include "vec.h"

typedef enum {
    TYPE_NAME_INTEGER,
    TYPE_NAME_STRING,
    TYPE_NAME_BOOLEAN,
    TYPE_NAME_TUPLE,
    TYPE_NAME_FUNCTION,
} ast_typename_type;

typedef enum {
    INTEGER_SIZE_8 = 1,
    INTEGER_SIZE_16 = 2,
    INTEGER_SIZE_32 = 4,
} ast_integer_size;

struct _ast_typename;

typedef VEC(struct _ast_typename*) vec_typename;

typedef struct {
} ast_typename_string, ast_typename_boolean;

typedef struct {
    bool is_signed;
    ast_integer_size size;
} ast_typename_integer;

typedef struct {
    vec_typename items;
} ast_typename_tuple;

typedef struct {
    vec_typename params;
    struct _ast_typename* return_type;
} ast_typename_function;

typedef struct _ast_typename {
    union {
        ast_typename_boolean boolean;
        ast_typename_integer integer;
        ast_typename_string string;
        ast_typename_tuple tuple;
        ast_typename_function function;
    } as;

    ast_typename_type type;
} ast_typename;

ast_typename* make_ast_typename(allocator_t* allocator, ast_typename_type type);

ast_typename* make_ast_typename_unit(allocator_t* allocator);

ast_typename* make_ast_typename_tuple(
    allocator_t* allocator, vec_typename items
);

ast_typename* make_ast_typename_integer(
    allocator_t* allocator, bool is_signed, ast_integer_size size
);

ast_typename* make_ast_typename_function(
    allocator_t* allocator, vec_typename params, ast_typename* return_type
);

void free_ast_typename(allocator_t* allocator, ast_typename* node);

#define AST_TYPENAME_WALKER(name, return_type, ctx_type)                            \
    struct _##name;                                                                 \
    typedef struct _##name {                                                        \
        return_type (*walk_integer_type)(struct _##name*, ast_typename_integer*);   \
        return_type (*walk_string_type)(struct _##name*, ast_typename_string*);     \
        return_type (*walk_boolean_type)(struct _##name*, ast_typename_boolean*);   \
        return_type (*walk_tuple_type)(struct _##name*, ast_typename_tuple*);       \
        return_type (*walk_function_type)(struct _##name*, ast_typename_function*); \
                                                                                    \
        ctx_type ctx;                                                               \
    } name;                                                                         \
                                                                                    \
    return_type name##_walk(name* walker, ast_typename* node) {                     \
        switch (node->type) {                                                       \
            case TYPE_NAME_INTEGER:                                                 \
                return walker->walk_integer_type(walker, &node->as.integer);        \
                                                                                    \
            case TYPE_NAME_BOOLEAN:                                                 \
                return walker->walk_boolean_type(walker, &node->as.boolean);        \
                                                                                    \
            case TYPE_NAME_STRING:                                                  \
                return walker->walk_string_type(walker, &node->as.string);          \
                                                                                    \
            case TYPE_NAME_TUPLE:                                                   \
                return walker->walk_tuple_type(walker, &node->as.tuple);            \
                                                                                    \
            case TYPE_NAME_FUNCTION:                                                \
                return walker->walk_function_type(walker, &node->as.function);      \
        }                                                                           \
    }

typedef struct _ast_param {
    token name;
    ast_typename* type;
    struct _ast_param* next;
} ast_param;

struct _ast_stmt_node;
typedef struct _ast_stmt_node ast_stmt_node;

typedef enum {
    AST_NUM,
    AST_BOOL,
    AST_STR,
    AST_IDEN,
    AST_BINARY,
    AST_UNARY,
    AST_CALL,
    AST_LAMBDA,
} ast_expr_node_type;

struct _ast_expr_node;

typedef VEC(struct _ast_expr_node*) vec_expr;

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

typedef struct {
    token_type op;
    struct _ast_expr_node* expr;
} ast_node_unary;

typedef struct {
    struct _ast_expr_node* function;
    vec_expr args;  //  vec of struct _ast_expr_node*
} ast_node_call;

typedef struct {
    struct _ast_stmt_node* body;
    ast_param* params;
    ast_typename* return_type;
} ast_node_lambda;

typedef struct _ast_expr_node {
    union {
        ast_node_num num;
        ast_node_bool boolean;
        ast_node_str str;
        ast_node_binary binary;
        ast_node_unary unary;
        ast_node_identifier identifier;
        ast_node_call call;
        ast_node_lambda lambda;
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
ast_expr_node* make_ast_unary(
    allocator_t* allocator, token_type op, ast_expr_node* expr
);
ast_expr_node* make_ast_call(
    allocator_t* allocator, ast_expr_node* function, vec_expr args
);
ast_expr_node* make_ast_lambda(
    allocator_t* allocator,
    ast_param* params,
    ast_stmt_node* body,
    ast_typename* return_type
);

void free_ast_expr(allocator_t* allocator, ast_expr_node* node);

#define AST_EXPR_WALKER(name, return_type, ctx_type)                     \
    struct _##name;                                                      \
    typedef struct _##name {                                             \
        return_type (*walk_binary)(struct _##name*, ast_node_binary*);   \
        return_type (*walk_unary)(struct _##name*, ast_node_unary*);     \
        return_type (*walk_call)(struct _##name*, ast_node_call*);       \
        return_type (*walk_num)(struct _##name*, ast_node_num*);         \
        return_type (*walk_iden)(struct _##name*, ast_node_identifier*); \
        return_type (*walk_str)(struct _##name*, ast_node_str*);         \
        return_type (*walk_bool)(struct _##name*, ast_node_bool*);       \
        return_type (*walk_lambda)(struct _##name*, ast_node_lambda*);   \
                                                                         \
        ctx_type ctx;                                                    \
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
            case AST_UNARY:                                              \
                return walker->walk_unary(walker, &node->unary);         \
                                                                         \
            case AST_CALL:                                               \
                return walker->walk_call(walker, &node->call);           \
                                                                         \
            case AST_STR:                                                \
                return walker->walk_str(walker, &node->str);             \
                                                                         \
            case AST_LAMBDA:                                             \
                return walker->walk_lambda(walker, &node->lambda);       \
        }                                                                \
    }

typedef enum {
    AST_EXPR_STMT,
    AST_VAR_DECL,
    AST_BLOCK,
    AST_IF_ELSE,
    AST_WHILE,
} ast_stmt_node_type;

typedef struct {
    ast_expr_node* expr;
} ast_node_expr_stmt;

typedef struct {
    token name;
    ast_typename* typename;
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

typedef struct {
    ast_expr_node* condition;
    struct _ast_stmt_node* body;
} ast_node_while;

struct _ast_stmt_node {
    union {
        ast_node_expr_stmt expr_stmt;
        ast_node_var_decl var_decl;
        ast_node_block block;
        ast_node_if_else if_else;
        ast_node_while while_;
    };
    ast_stmt_node_type type;

    struct _ast_stmt_node* next;
};

ast_stmt_node* make_ast_expr_stmt(allocator_t* allocator, ast_expr_node* expr);
ast_stmt_node* make_ast_var_decl(
    allocator_t* allocator,
    token name,
    ast_typename* typename,
    ast_expr_node* value,
    bool mut
);
ast_stmt_node* make_ast_block(allocator_t* allocator, ast_stmt_node* body);

ast_stmt_node* make_ast_if_else(
    allocator_t* allocator,
    ast_expr_node* condition,
    ast_stmt_node* body,
    ast_stmt_node* else_body
);

ast_stmt_node* make_ast_while(
    allocator_t* allocator, ast_expr_node* condition, ast_stmt_node* body
);

void free_ast_stmt(allocator_t* allocator, ast_stmt_node* node);

#define AST_STMT_WALKER(name, return_type, ctx_type)                         \
    struct _##name;                                                          \
    typedef struct _##name {                                                 \
        return_type (*walk_expr_stmt)(struct _##name*, ast_node_expr_stmt*); \
        return_type (*walk_var_decl)(struct _##name*, ast_node_var_decl*);   \
        return_type (*walk_block)(struct _##name*, ast_node_block*);         \
        return_type (*walk_if_else)(struct _##name*, ast_node_if_else*);     \
        return_type (*walk_while)(struct _##name*, ast_node_while*);         \
                                                                             \
        ctx_type ctx;                                                        \
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
            case AST_WHILE: {                                                \
                return walker->walk_while(walker, &node->while_);            \
            }                                                                \
        }                                                                    \
    }

typedef enum {
    AST_FN,
    AST_STRUCT,
} ast_item_node_type;

struct _ast_item_node;

typedef struct {
    token name;
    ast_param* params;
    ast_stmt_node* body;
    ast_typename* return_type;
} ast_node_function;

typedef struct _ast_item_node {
    union {
        ast_node_function function;
    };
    ast_item_node_type type;

    struct _ast_item_node* next;
} ast_item_node;

ast_param* make_ast_param(
    allocator_t* allocator, token name, ast_typename* type
);
ast_item_node* make_ast_function(
    allocator_t* allocator,
    token name,
    ast_param* params,
    ast_stmt_node* body,
    ast_typename* return_type
);

void free_ast_item(allocator_t* allocator, ast_item_node* node);

#define AST_ITEM_WALKER(name, return_type, ctx_type)                       \
    struct _##name;                                                        \
    typedef struct _##name {                                               \
        return_type (*walk_function)(struct _##name*, ast_node_function*); \
                                                                           \
        ctx_type ctx;                                                      \
    } name;                                                                \
                                                                           \
    return_type name##_walk(name* walker, ast_item_node* node) {           \
        switch (node->type) {                                              \
            case AST_FN: {                                                 \
                return walker->walk_function(walker, &node->function);     \
            }                                                              \
        }                                                                  \
    }

#endif  // AST_H
