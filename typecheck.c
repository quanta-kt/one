#include "typecheck.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct _typeres typeres;

typedef struct {
    token name;
    typeres* type;
} symbol;

typedef VEC(symbol) vec_symbol;

typedef struct _environment {
    vec_symbol symbols;
    struct _environment* prev;
} environment;

typedef struct {
    allocator_t* allocator;
    environment* env;
} tc_ctx;

AST_EXPR_WALKER(ast_expr_tc_t, typeres*, tc_ctx*)
AST_STMT_WALKER(ast_stmt_tc_t, int, tc_ctx*)
AST_ITEM_WALKER(ast_item_tc_t, int, tc_ctx*)

static ast_item_tc_t make_item_tc(tc_ctx* ctx);
static ast_stmt_tc_t make_stmt_tc(tc_ctx* ctx);
static ast_expr_tc_t make_expr_tc(tc_ctx* ctx);

static void environment_push(allocator_t* allocator, environment** env);
static void environment_pop(allocator_t* allocator, environment** env);

bool typecheck(allocator_t* allocator, ast_item_node* ast) {
    tc_ctx ctx = (tc_ctx){.allocator = allocator, .env = NULL};

    // global environment
    environment_push(allocator, &ctx.env);

    bool ret = true;

    ast_item_tc_t tc = make_item_tc(&ctx);

    ast_item_node* curr = ast;
    while (curr != NULL) {
        ret &= ast_item_tc_t_walk(&tc, curr);
        curr = curr->next;
    }

    // global environment
    environment_pop(allocator, &ctx.env);

    return ret;
}

static bool typecheck_stmt(tc_ctx* ctx, ast_stmt_node* stmt) {
    ast_stmt_tc_t walker = make_stmt_tc(ctx);
    return ast_stmt_tc_t_walk(&walker, stmt);
}

static bool typecheck_stmt_list(tc_ctx* ctx, ast_stmt_node* stmts) {
    ast_stmt_tc_t walker = make_stmt_tc(ctx);
    bool ret = true;

    ast_stmt_node* curr = stmts;
    while (curr != NULL) {
        ret &= ast_stmt_tc_t_walk(&walker, curr);
        curr = curr->next;
    }

    return ret;
}

static typeres* typecheck_expr(tc_ctx* ctx, ast_expr_node* expr) {
    ast_expr_tc_t walker = make_expr_tc(ctx);
    typeres* ret = ast_expr_tc_t_walk(&walker, expr);
    return ret;
}

typedef VEC(typeres*) vec_typeres;

typedef enum {
    TYPE_RES_INTEGER,
    TYPE_RES_STRING,
    TYPE_RES_BOOLEAN,
    TYPE_RES_TUPLE,
    TYPE_RES_FUNCTION,

    TYPE_RES_UNKNOWN,
} typeres_type;

// A type resolution, i.e. a resolved type of an expression or a symbol.
typedef struct _typeres {
    union {
        struct {
            bool is_signed;
            ast_integer_size size;

            /**
             * Sometimes we don't directly know from the context what size ang
             * sign a number node is. Consider the statement:
             *
             *     let num: i16 = 44;
             *
             * Here it is clear that the name 'num' itself has type 'i16', but
             * the AST node holding the number '44' does not have any type
             * information to it. We are therefore unable to concretely resolve
             * the size and sign of the number literals without explicit type,
             * like in this case.
             *
             * For number literals that don't explicitly specify their type, we
             * lax the type-checking until we discover what the concrete type
             * should be. In this case, we discover the concreate type when
             * walking the assignment node.
             *
             * Until we can know the "concrete" type of a number, we assume its
             * an i32. Therefore, in the following code:
             *
             *     let num = 44;
             *
             * 'num' defaults to i32, as the literal 44 by default has type i32.
             *
             * This field tells us if this type was resolved with default sign
             * and size.
             */
            bool default_until_inferred;
        } integer;

        struct {
            vec_typeres params;
            typeres* return_type;
        } function;

        struct {
            vec_typeres items;
        } tuple;
    };

    // What base type are we?
    typeres_type type;

    // Was there a type error at this node?
    bool is_err;
} typeres;

static typeres* make_typeres(
    allocator_t* allocator, bool is_err, ast_typename_type type
) {
    typeres* res = ALLOC(allocator, typeres);
    res->is_err = is_err;
    res->type = type;
    return res;
}

static typeres* make_typeres_function(
    allocator_t* allocator, vec_typeres params, typeres* return_type
) {
    typeres* res = make_typeres(allocator, false, TYPE_RES_FUNCTION);
    res->function.params = params;
    res->function.return_type = return_type;
    return res;
}

static typeres* make_typeres_integer(
    allocator_t* allocator,
    bool is_signed,
    ast_integer_size size,
    bool is_implicit
) {
    typeres* res = make_typeres(allocator, false, TYPE_RES_INTEGER);
    res->integer.is_signed = is_signed;
    res->integer.size = size;
    res->integer.default_until_inferred = is_implicit;

    return res;
}

static vec_typeres vec_typeres_dup(allocator_t* allocator, vec_typeres* src);

static typeres* typeres_dup(allocator_t* allocator, typeres* src) {
    typeres* res = ALLOC(allocator, typeres);

    res->is_err = src->is_err;
    res->type = src->type;

    if (src->type == TYPE_RES_INTEGER) {
        res->integer.is_signed = src->integer.is_signed;
        res->integer.size = src->integer.size;
        res->integer.default_until_inferred =
            src->integer.default_until_inferred;
    }

    if (src->type == TYPE_RES_TUPLE) {
        res->tuple.items = vec_typeres_dup(allocator, &src->tuple.items);
    }

    if (src->type == TYPE_RES_FUNCTION) {
        res->function.return_type =
            typeres_dup(allocator, src->function.return_type);

        res->function.params =
            vec_typeres_dup(allocator, &src->function.params);
    }

    return res;
}

/**
 * Duplicates all typeres in src
 */
static vec_typeres vec_typeres_dup(allocator_t* allocator, vec_typeres* src) {
    vec_typeres ret = vec_make(allocator);

    typeres** item;
    vec_foreach(src, item) {
        typeres* dup_param = typeres_dup(allocator, *item);
        vec_push(&ret, &dup_param);
    }

    return ret;
}

static bool vec_typeres_is_eq(vec_typeres* left, vec_typeres* right);

/**
 * Checks if two types are equal (i.o.w compatible with each other)
 */
static bool typeres_is_eq(typeres* left, typeres* right) {
    if (left->type != right->type) {
        return false;
    }

    if (left->type == TYPE_RES_INTEGER) {
        return left->integer.is_signed == right->integer.is_signed &&
                   left->integer.size == right->integer.size ||

               // if the size and sign on at least of the types are implicit, we
               // overlook the details
               left->integer.default_until_inferred ||
               right->integer.default_until_inferred;
    }

    if (left->type == TYPE_RES_TUPLE) {
        return vec_typeres_is_eq(&left->tuple.items, &right->tuple.items);
    }

    if (left->type == TYPE_RES_FUNCTION) {
        // Two function types are equivalent if their return type and parameters
        // are.
        return typeres_is_eq(
                   left->function.return_type,
                   right->function.return_type
               ) &&
               vec_typeres_is_eq(
                   &left->function.params,
                   &right->function.params
               );
    }

    // For other types, only equality of the base type matters.
    return true;
}

/**
 * Tests two typeres vectors for equality.
 */
static bool vec_typeres_is_eq(vec_typeres* left, vec_typeres* right) {
    if (left->len != right->len) {
        return false;
    }

    // since we know the length of both the vectors at this point
    // is equal, the indices should match up
    for (size_t i = 0; i < left->len; i++) {
        typeres* item_left = left->items[i];
        typeres* item_right = right->items[i];

        if (!typeres_is_eq(item_left, item_right)) {
            return false;
        }
    }

    return true;
}

/**
 * If `type` is implicitly assumed to be the default signed/sized number,
 * change its sign/size to match that of `infer_from`.
 * This is the "type inference" stage of the number literals.
 *
 * The "try" in the function name implies that the type is not guaranteed to
 * have a concrete size and sign this is called in case where infer_from itself
 * is implicitly assumed to be the default sign/size.
 */
static void typeres_try_infer_number_type(
    typeres* type, const typeres* infer_from
) {
    if (type->type != infer_from->type) {
        return;
    }

    if (type->type != TYPE_RES_INTEGER) {
        return;
    }

    // we already know the type
    if (!type->integer.default_until_inferred) {
        return;
    }

    type->integer.is_signed = infer_from->integer.is_signed;
    type->integer.size = infer_from->integer.size;
    type->integer.default_until_inferred =
        infer_from->integer.default_until_inferred;
}

/**
 * Same as typeres_try_infer_number_type but marks the implicitly indefred
 * default sign and size as final  if type of `infer_from` is itself not
 * concretely known.
 *
 * In other words, unlike typepres_try_infer_number, this variant guarantees
 * that after the call, the number type will have a concrete type.
 */
static void typeres_infer_number_type(
    typeres* type, const typeres* infer_from
) {
    typeres_try_infer_number_type(type, infer_from);

    if (type->type != TYPE_RES_INTEGER) {
        return;
    }

    // we already know the type
    if (!type->integer.default_until_inferred) {
        return;
    }

    type->integer.default_until_inferred = false;
}

static typeres* make_typeres_from_ast(
    allocator_t* allocator, ast_typename* typename
);

static vec_typeres make_typeres_vec_from_ast_params(
    allocator_t* allocator, ast_param* params
) {
    vec_typeres ret = vec_make(allocator);

    ast_param* curr = params;
    while (curr != NULL) {
        typeres* param_type = make_typeres_from_ast(allocator, curr->type);
        vec_push(&ret, &param_type);
        curr = curr->next;
    }

    return ret;
}

static typeres* make_typeres_from_ast(
    allocator_t* allocator, ast_typename* typename
) {
    typeres* res = ALLOC(allocator, typeres);

    switch (typename->type) {
        case TYPE_NAME_BOOLEAN: {
            res->type = TYPE_RES_BOOLEAN;
            break;
        }
        case TYPE_NAME_INTEGER: {
            res->type = TYPE_RES_INTEGER;
            res->integer.is_signed = typename->as.integer.is_signed;
            res->integer.size = typename->as.integer.size;
            break;
        }
        case TYPE_NAME_STRING: {
            res->type = TYPE_RES_STRING;
            break;
        }

        case TYPE_NAME_TUPLE: {
            res->type = TYPE_RES_TUPLE;

            res->tuple.items = (vec_typeres)vec_make(allocator);
            ast_typename** item;
            vec_foreach(&typename->as.tuple.items, item) {
                typeres* item_res = make_typeres_from_ast(allocator, *item);
                vec_push(&res->tuple.items, &item_res);
            }

            break;
        }

        case TYPE_NAME_FUNCTION: {
            res->type = TYPE_RES_FUNCTION;

            res->function.params = (vec_typeres)vec_make(allocator);
            ast_typename** param;
            vec_foreach(&typename->as.function.params, param) {
                typeres* param_res = make_typeres_from_ast(allocator, *param);
                vec_push(&res->function.params, &param_res);
            }

            res->function.return_type = make_typeres_from_ast(
                allocator,
                typename->as.function.return_type
            );

            break;
        }
    }

    return res;
}

static void free_typeres(allocator_t* allocator, typeres* res) {
    switch (res->type) {
        case TYPE_RES_BOOLEAN:
        case TYPE_RES_STRING:
        case TYPE_RES_INTEGER:
            break;

        case TYPE_RES_FUNCTION: {
            free_typeres(allocator, res->function.return_type);

            typeres** param;
            vec_foreach(&res->function.params, param) {
                free_typeres(allocator, *param);
            }
            vec_free(&res->function.params);
            break;
        }
    }

    FREE(allocator, res, typeres);
}

static void environment_push(allocator_t* allocator, environment** env) {
    environment* next = ALLOC(allocator, environment);
    *next = (environment){
        .prev = *env,
        .symbols = (vec_symbol)vec_make(allocator),
    };

    *env = next;
}

static void environment_pop(allocator_t* allocator, environment** env) {
    if (*env == NULL) {
        return;
    }

    environment* prev = (*env)->prev;

    // since we own the typeres, we must free it here
    symbol* sym;
    vec_foreach(&(*env)->symbols, sym) { free_typeres(allocator, sym->type); }

    vec_free(&(*env)->symbols);
    FREE(allocator, *env, environment);

    *env = prev;
}

static void environment_put_symbol(
    environment* env, token name, typeres* type
) {
    symbol sym = (symbol){.name = name, .type = type};
    vec_push(&env->symbols, &sym);
}

static typeres* environment_lookup_symbol(environment* env, token name) {
    environment* curr = env;

    while (curr != NULL) {
        symbol* sym;
        sym = (&curr->symbols)->items;
        for (size_t i = 0; i < (&curr->symbols)->len; i++, (sym)++) {
            if (name.span_size != sym->name.span_size) {
                continue;
            }

            if (memcmp(sym->name.span, name.span, name.span_size) == 0) {
                return sym->type;
            }
        }

        curr = curr->prev;
    }

    return NULL;
}

static void report_type_err(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fputc('\n', stderr);
}

// Expression walker

static typeres* typecheck_op_assign(
    allocator_t* allocator, typeres* left, typeres* right
) {
    if (right->type == left->type) {
        // infer from the variable's type
        typeres_infer_number_type(right, left);
    }

    typeres* ret = typeres_dup(allocator, left);
    ret->is_err = !typeres_is_eq(left, right);

    if (ret->is_err) {
        report_type_err("incompatible assignment");
    }

    return ret;
}

static typeres* typecheck_op_eq(
    allocator_t* allocator, typeres* left, typeres* right
) {
    typeres_try_infer_number_type(left, right);
    typeres_try_infer_number_type(right, left);

    typeres* ret =
        make_typeres(allocator, !typeres_is_eq(left, right), TYPE_RES_BOOLEAN);

    if (ret->is_err) {
        report_type_err("incompatible comparision");
    }

    return ret;
}

static typeres* typecheck_op_and(
    allocator_t* allocator, typeres* left, typeres* right
) {
    typeres* ret = make_typeres(
        allocator,
        left->type != TYPE_RES_BOOLEAN || right->type != TYPE_RES_BOOLEAN,
        TYPE_RES_BOOLEAN
    );
    if (ret->is_err) {
        report_type_err("&& can only be applied to boolean operands");
    }

    return ret;
}

static typeres* typecheck_op_or(
    allocator_t* allocator, typeres* left, typeres* right
) {
    typeres* ret = make_typeres(
        allocator,
        left->type != TYPE_RES_BOOLEAN || right->type != TYPE_RES_BOOLEAN,
        TYPE_RES_BOOLEAN
    );
    if (ret->is_err) {
        report_type_err("|| can only be applied to boolean operands");
    }

    return ret;
}

static typeres* typecheck_op_plus(
    allocator_t* allocator, typeres* left, typeres* right
) {
    typeres_try_infer_number_type(left, right);
    typeres_try_infer_number_type(right, left);

    bool err = left->type != right->type;
    typeres_type type = TYPE_RES_UNKNOWN;

    if (err) {
        report_type_err("incompatible operands for '+'");
    }

    typeres* ret = typeres_dup(allocator, left);

    switch (left->type) {
        case TYPE_RES_INTEGER:
        case TYPE_RES_STRING:
            break;

        default:
            err = true;
            report_type_err("'+' is only supported for numbers and strings");
            break;
    }

    ret->is_err = err;

    return ret;
}

static typeres* typecheck_op_minus(
    allocator_t* allocator, typeres* left, typeres* right
) {
    typeres_try_infer_number_type(left, right);
    typeres_try_infer_number_type(right, left);

    typeres* ret = typeres_dup(allocator, left);
    ret->is_err = left->type != right->type || left->type != TYPE_RES_INTEGER;

    if (ret->is_err) {
        report_type_err(
            "incompatible operands for '-': only supported for numbers"
        );
    }

    return ret;
}

static typeres* typecheck_op_bitwise_or(
    allocator_t* allocator, typeres* left, typeres* right
) {
    typeres_try_infer_number_type(left, right);
    typeres_try_infer_number_type(right, left);

    typeres* ret = typeres_dup(allocator, left);
    ret->is_err = left->type != right->type || left->type != TYPE_RES_INTEGER;

    if (ret->is_err) {
        report_type_err(
            "incompatible operands for '|': only supported for numbers"
        );
    }

    return ret;
}

static typeres* typecheck_op_bitwise_and(
    allocator_t* allocator, typeres* left, typeres* right
) {
    typeres_try_infer_number_type(left, right);
    typeres_try_infer_number_type(right, left);

    typeres* ret = typeres_dup(allocator, left);
    ret->is_err = left->type != right->type || left->type != TYPE_RES_INTEGER;

    if (ret->is_err) {
        report_type_err(
            "incompatible operands for '&': only supported for numbers"
        );
    }

    return ret;
}

static typeres* typecheck_op_xor(
    allocator_t* allocator, typeres* left, typeres* right
) {
    typeres_try_infer_number_type(left, right);
    typeres_try_infer_number_type(right, left);

    typeres* ret = typeres_dup(allocator, left);
    ret->is_err = left->type != right->type || left->type != TYPE_RES_INTEGER;

    if (ret->is_err) {
        report_type_err(
            "incompatible operands for '^': only supported for numbers"
        );
    }

    return ret;
}

static typeres* typecheck_op_gt(
    allocator_t* allocator, typeres* left, typeres* right
) {
    typeres_try_infer_number_type(left, right);
    typeres_try_infer_number_type(right, left);

    typeres* ret = make_typeres(
        allocator,
        left->type != right->type || left->type != TYPE_RES_INTEGER,
        TYPE_RES_BOOLEAN
    );
    if (ret->is_err) {
        report_type_err(
            "incompatible operands for '>': can only compare numbers"
        );
    }

    return ret;
}

static typeres* typecheck_op_lt(
    allocator_t* allocator, typeres* left, typeres* right
) {
    typeres_try_infer_number_type(left, right);
    typeres_try_infer_number_type(right, left);

    typeres* ret = make_typeres(
        allocator,
        left->type != right->type || left->type != TYPE_RES_INTEGER,
        TYPE_RES_BOOLEAN
    );
    if (ret->is_err) {
        report_type_err(
            "incompatible operands for '<': can only compare numbers"
        );
    }

    return ret;
}

static typeres* typecheck_op_mod(

    allocator_t* allocator, typeres* left, typeres* right
) {
    typeres_try_infer_number_type(left, right);
    typeres_try_infer_number_type(right, left);

    typeres* ret = typeres_dup(allocator, left);
    ret->is_err = left->type != right->type || left->type != TYPE_RES_INTEGER;

    if (ret->is_err) {
        report_type_err(
            "incompatible operands for '%%': only supported for numbers"
        );
    }

    return ret;
}

static typeres* typecheck_op_mul(
    allocator_t* allocator, typeres* left, typeres* right
) {
    typeres_try_infer_number_type(left, right);
    typeres_try_infer_number_type(right, left);

    typeres* ret = typeres_dup(allocator, left);
    ret->is_err = left->type != right->type || left->type != TYPE_RES_INTEGER;

    if (ret->is_err) {
        report_type_err(
            "incompatible operands for '*': only supported for numbers"
        );
    }

    return ret;
}

static typeres* typecheck_op_div(
    allocator_t* allocator, typeres* left, typeres* right
) {
    typeres_try_infer_number_type(left, right);
    typeres_try_infer_number_type(right, left);

    typeres* ret = typeres_dup(allocator, left);
    ret->is_err = left->type != right->type || left->type != TYPE_RES_INTEGER;

    if (ret->is_err) {
        report_type_err(
            "incompatible operands for '/': only supported for numbers"
        );
    }

    return ret;
}

typedef typeres* typecheck_op_fn(
    allocator_t* allocator, typeres* left, typeres* right
);

struct {
    token_type op;
    typecheck_op_fn* fn;
} typecheck_op_table[] = {
    {.op = TOK_AMP, .fn = typecheck_op_bitwise_and},
    {.op = TOK_AND, .fn = typecheck_op_and},
    {.op = TOK_ASSIGN, .fn = typecheck_op_assign},
    {.op = TOK_CARET, .fn = typecheck_op_xor},
    {.op = TOK_DIV, .fn = typecheck_op_div},
    {.op = TOK_EQ, .fn = typecheck_op_eq},
    {.op = TOK_GT, .fn = typecheck_op_gt},
    {.op = TOK_LT, .fn = typecheck_op_lt},
    {.op = TOK_MINUS, .fn = typecheck_op_minus},
    {.op = TOK_MUL, .fn = typecheck_op_mul},
    {.op = TOK_OR, .fn = typecheck_op_or},
    {.op = TOK_PERC, .fn = typecheck_op_mod},
    {.op = TOK_PIPE, .fn = typecheck_op_bitwise_or},
    {.op = TOK_PLUS, .fn = typecheck_op_plus},
};

const size_t typecheck_op_table_len =
    sizeof(typecheck_op_table) / sizeof(typecheck_op_table[0]);

static typeres* walk_binary(ast_expr_tc_t* self, ast_node_binary* expr) {
    typeres* ret;

    typeres* left = ast_expr_tc_t_walk(self, expr->left);
    typeres* right = ast_expr_tc_t_walk(self, expr->right);

    typecheck_op_fn* fn = NULL;
    for (size_t i = 0; i < typecheck_op_table_len; i++) {
        if (typecheck_op_table[i].op == expr->op) {
            fn = typecheck_op_table[i].fn;
            break;
        }
    }

    if (fn == NULL) {
        ret = make_typeres(self->ctx->allocator, true, TYPE_RES_UNKNOWN);

        report_type_err("BUG: unkown binary operator");
    } else {
        ret = fn(self->ctx->allocator, left, right);
    }

cleanup:
    free_typeres(self->ctx->allocator, left);
    free_typeres(self->ctx->allocator, right);

    return ret;
}

typeres* walk_bool(ast_expr_tc_t* self, ast_node_bool* expr) {
    typeres* res = make_typeres(self->ctx->allocator, false, TYPE_RES_BOOLEAN);
    return res;
}

typeres* walk_call(ast_expr_tc_t* self, ast_node_call* expr) {
    typeres* res;

    typeres* fn_res = ast_expr_tc_t_walk(self, expr->function);

    if (fn_res->type != TYPE_RES_FUNCTION) {
        report_type_err("can only call function types");

        res = make_typeres(self->ctx->allocator, true, TYPE_RES_UNKNOWN);
        goto cleanup;
    }

    res = typeres_dup(self->ctx->allocator, fn_res->function.return_type);

    if (fn_res->function.params.len > expr->args.len) {
        res->is_err = true;
        report_type_err("insufficient arguments to function");
    } else if (fn_res->function.params.len < expr->args.len) {
        res->is_err = true;
        report_type_err("too many arguments to function");
    }

    vec_typeres* params = &fn_res->function.params;
    vec_expr* args = &expr->args;

    // minimum of both lengths
    size_t len = params->len < args->len ? params->len : args->len;

    for (size_t i = 0; i < len; i++) {
        ast_expr_node** arg = vec_get(args, i);

        typeres* arg_res = ast_expr_tc_t_walk(self, *arg);
        typeres** param = vec_get(params, i);

        if (arg_res->type != (*param)->type) {
            res->is_err = true;
            // TODO: report error: param and arg type mismatch
        }

        free_typeres(self->ctx->allocator, arg_res);
    }

cleanup:
    free_typeres(self->ctx->allocator, fn_res);
    return res;
}

typeres* walk_iden(ast_expr_tc_t* self, ast_node_identifier* expr) {
    token name = (token){
        .type = TOK_IDEN,
        .span = expr->start,
        .span_size = expr->len,
    };

    typeres* type = environment_lookup_symbol(self->ctx->env, name);

    typeres* ret;

    if (type == NULL) {
        report_type_err("undeclared variable '%.*s'", expr->len, expr->start);
        ret = make_typeres(self->ctx->allocator, true, TYPE_RES_UNKNOWN);
    } else {
        // we want the caller to own the returned typeres, so we duplicate it
        ret = typeres_dup(self->ctx->allocator, type);
    }

    return ret;
}

typeres* walk_lambda(ast_expr_tc_t* self, ast_node_lambda* expr) {
    environment_push(self->ctx->allocator, &self->ctx->env);

    vec_typeres params =
        make_typeres_vec_from_ast_params(self->ctx->allocator, expr->params);
    typeres* return_type =
        make_typeres_from_ast(self->ctx->allocator, expr->return_type);
    typeres* ret =
        make_typeres_function(self->ctx->allocator, params, return_type);

    bool passes = true;

    if (expr->body != NULL) {
        ast_param* curr = expr->params;
        while (curr != NULL) {
            typeres* param_type =
                make_typeres_from_ast(self->ctx->allocator, curr->type);
            environment_put_symbol(self->ctx->env, curr->name, param_type);
            curr = curr->next;
        }

        passes = typecheck_stmt_list(self->ctx, expr->body);
    }

    ret->is_err |= !passes;

    environment_pop(self->ctx->allocator, &self->ctx->env);

    return ret;
}

typeres* walk_num(ast_expr_tc_t* self, ast_node_num* expr) {
    return make_typeres_integer(
        self->ctx->allocator,
        true,
        INTEGER_SIZE_32,
        // we don't know the actual sign an size yet, i32 is assumed
        true
    );
}

typeres* walk_str(ast_expr_tc_t* self, ast_node_str* expr) {
    return make_typeres(self->ctx->allocator, false, TYPE_RES_STRING);
}

typeres* walk_unary(ast_expr_tc_t* self, ast_node_unary* expr) {
    typeres* res = ast_expr_tc_t_walk(self, expr->expr);

    // FIXME: switch over the actual op and determine the resultant type.
    // report type errors.
    //
    // currently, this is sufficient as we do not have any unary operators
    // that produce a value of type different than their operand.
    return res;
}

ast_expr_tc_t make_expr_tc(tc_ctx* ctx) {
    ast_expr_tc_t expr_tc = (ast_expr_tc_t){
        .walk_binary = walk_binary,
        .walk_bool = walk_bool,
        .walk_call = walk_call,
        .walk_iden = walk_iden,
        .walk_lambda = walk_lambda,
        .walk_num = walk_num,
        .walk_str = walk_str,
        .walk_unary = walk_unary,
        .ctx = ctx,
    };

    return expr_tc;
}

// Statement walker

int walk_block(ast_stmt_tc_t* self, ast_node_block* stmt) {
    environment_push(self->ctx->allocator, &self->ctx->env);

    ast_stmt_node* current = stmt->body;
    bool ret = true;

    while (current != NULL) {
        ret &= ast_stmt_tc_t_walk(self, current);
        current = current->next;
    }

    environment_pop(self->ctx->allocator, &self->ctx->env);

    return ret;
}

int walk_expr_stmt(ast_stmt_tc_t* self, ast_node_expr_stmt* stmt) {
    typeres* res = typecheck_expr(self->ctx, stmt->expr);
    bool ret = !res->is_err;

    free_typeres(self->ctx->allocator, res);

    return ret;
}

int walk_if_else(ast_stmt_tc_t* self, ast_node_if_else* stmt) {
    typeres* res = typecheck_expr(self->ctx, stmt->condition);
    bool ret = !res->is_err;

    ret = res->type == TYPE_RES_BOOLEAN;
    if (!ret) {
        report_type_err("if statement must follow a boolean expression");
    }

    free_typeres(self->ctx->allocator, res);

    if (stmt->else_body) {
        ret &= ast_stmt_tc_t_walk(self, stmt->else_body);
    }

    return ret;
}

int walk_var_decl(ast_stmt_tc_t* self, ast_node_var_decl* stmt) {
    int ret = true;

    typeres* value_type =
        stmt->value != NULL
            ? typecheck_expr(self->ctx, stmt->value)
            : make_typeres(self->ctx->allocator, false, TYPE_RES_UNKNOWN);

    typeres* variable_type =
        stmt->typename != NULL
            ? make_typeres_from_ast(self->ctx->allocator, stmt->typename)
            // when explicit type is missing, we use the expression's type
            : typeres_dup(self->ctx->allocator, value_type);

    typeres_infer_number_type(variable_type, value_type);

    environment_put_symbol(self->ctx->env, stmt->name, variable_type);

    ret = !value_type->is_err;

    if (stmt->value != NULL && !typeres_is_eq(variable_type, value_type)) {
        report_type_err("incompatible assignment at variable initialization");
        ret = false;
    }

    free_typeres(self->ctx->allocator, value_type);

    return ret;
}

int walk_while(ast_stmt_tc_t* self, ast_node_while* stmt) {
    typeres* res = typecheck_expr(self->ctx, stmt->condition);

    bool ret = !res->is_err;
    if (!ret) {
        report_type_err("condition in 'while' must be a boolean");
    }

    free_typeres(self->ctx->allocator, res);

    if (stmt->body) {
        ret &= ast_stmt_tc_t_walk(self, stmt->body);
    }

    return ret;
}

ast_stmt_tc_t make_stmt_tc(tc_ctx* ctx) {
    ast_stmt_tc_t stmt_tc = (ast_stmt_tc_t){
        .walk_block = walk_block,
        .walk_expr_stmt = walk_expr_stmt,
        .walk_if_else = walk_if_else,
        .walk_var_decl = walk_var_decl,
        .walk_while = walk_while,
        .ctx = ctx,
    };

    return stmt_tc;
}

// Item walker

int walk_function(ast_item_tc_t* self, ast_node_function* fn) {
    vec_typeres params =
        make_typeres_vec_from_ast_params(self->ctx->allocator, fn->params);
    typeres* return_type =
        make_typeres_from_ast(self->ctx->allocator, fn->return_type);
    typeres* fn_type =
        make_typeres_function(self->ctx->allocator, params, return_type);

    // FIXME: We want to be able to call other functions regardless of whether
    // they were declared before or after the current function. This will
    // probably require us to take two separate passes
    environment_put_symbol(self->ctx->env, fn->name, fn_type);

    environment_push(self->ctx->allocator, &self->ctx->env);

    ast_param* curr = fn->params;
    while (curr != NULL) {
        typeres* param_type =
            make_typeres_from_ast(self->ctx->allocator, curr->type);
        environment_put_symbol(self->ctx->env, curr->name, param_type);
        curr = curr->next;
    }

    if (fn->body != NULL) {
        return typecheck_stmt_list(self->ctx, fn->body);
    }

    environment_pop(self->ctx->allocator, &self->ctx->env);

    return true;
}

ast_item_tc_t make_item_tc(tc_ctx* ctx) {
    ast_item_tc_t item_tc = (ast_item_tc_t){
        .walk_function = walk_function,
        .ctx = ctx,
    };

    return item_tc;
}
