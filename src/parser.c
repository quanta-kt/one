#include "parser.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "error_printer.h"

typedef struct {
    lexer_t lexer;
    allocator_t* allocator;

    token curr;
    token prev;

    ast_item_node* item_head;
    ast_item_node* item_tail;

    /* Was there *any* error reported while parsing the code? */
    bool has_error;
    error_printer_t error_printer;
} parser_t;

static ast_typename* function_typename(parser_t* parser);
static ast_typename* typename(parser_t* parser);

static ast_item_node* item(parser_t* parser);
static ast_item_node* function_decl(parser_t* parser);

static ast_stmt_node* stmt(parser_t* parser);
static ast_stmt_node* var_decl(parser_t* parser);
static ast_stmt_node* block(parser_t* parser);
static ast_stmt_node* if_else(parser_t* parser);
static ast_stmt_node* while_(parser_t* parser);
static ast_stmt_node* expr_stmt(parser_t* parser);

static ast_expr_node* expr(parser_t* parser);
static ast_expr_node* assign(parser_t* parser);
static ast_expr_node* or_op(parser_t* parser);
static ast_expr_node* and_op(parser_t* parser);
static ast_expr_node* bitwise_or(parser_t* parser);
static ast_expr_node* bitwise_xor(parser_t* parser);

static ast_expr_node* bitwise_and(parser_t* parser);
static ast_expr_node* equality(parser_t* parser);
static ast_expr_node* comparision(parser_t* parser);
static ast_expr_node* term(parser_t* parser);
static ast_expr_node* factor(parser_t* parser);
static ast_expr_node* unary(parser_t* parser);
static ast_expr_node* function_call(parser_t* parser);
static ast_expr_node* primary(parser_t* parser);
static ast_expr_node* number(parser_t* parser);
static ast_expr_node* iden(parser_t* parser);
static ast_expr_node* group(parser_t* parser);
static ast_expr_node* str(parser_t* parser);
static ast_expr_node* boolean(parser_t* parser);
static ast_expr_node* lambda(parser_t* parser);

/**
 * Takes a pass at the source code to check if curly braces are balanced.
 *
 * \p{lexer} An initalized lexer containing the source code. This wil be
 *           consumed.
 *
 * @returns true if curly braces are balanced, false otherwise.
 */
static bool validate_curly_brace_balance(error_printer_t error_printer, lexer_t lexer);

static void syntax_error_at_current(parser_t* parser, char const* fmt, ...) {
    parser->has_error = true;
    va_list args;
    va_start(args, fmt);
    parser->error_printer.error(parser->lexer.src, &parser->curr.span, ERROR_KIND_SYNTAX,
                                 fmt, args);
    va_end(args);
}

static void syntax_error_at_current_and_die(parser_t* parser, char const* fmt, ...) {
    parser->has_error = true;
    va_list args;
    va_start(args, fmt);
    parser->error_printer.error(parser->lexer.src, &parser->curr.span, ERROR_KIND_SYNTAX,
                                 fmt, args);
    va_end(args);
    exit(1);
}

static void syntax_error_at_previous(parser_t* parser, char const* fmt, ...) {
    parser->has_error = true;
    va_list args;
    va_start(args, fmt);
    parser->error_printer.error(parser->lexer.src, &parser->prev.span, ERROR_KIND_SYNTAX,
                                 fmt, args);
    va_end(args);
}

static void syntax_error_at_previous_and_die(parser_t* parser, char const* fmt, ...) {
    parser->has_error = true;
    va_list args;
    va_start(args, fmt);
    parser->error_printer.error(parser->lexer.src, &parser->prev.span, ERROR_KIND_SYNTAX,
                                 fmt, args);
    va_end(args);
    exit(1);
}

static void syntax_error_at_span(
    error_printer_t error_printer, char* src, span_info* span,
    char const* fmt, ...
) {
    va_list args;
    va_start(args, fmt);
    error_printer.error(src, span, ERROR_KIND_SYNTAX,
                        fmt, args);
    va_end(args);
}

static void lex_e_print(lexer_t* lex, lex_error e, error_printer_t error_printer) {
    switch (e.type) {
        case LEX_ERR_UNEXPECTED_CHAR: {
            syntax_error_at_span(
                    error_printer,
                    lex->src,
                    &e.span,
                    "Unexpected character: '%c'",
                    *e.span.span
            );
            break;
        }

        case LEX_ERR_UNTERMINATED_STRING: {
            syntax_error_at_span(
                    error_printer,
                    lex->src,
                    &e.span,
                    "Unterminated string: %.*s",
                    (int)e.span.span_size,
                    e.span.span
            );
            break;
        }
    }

    exit(1);
}

static parser_t make_parser(allocator_t* allocator, lexer_t lexer,
                            error_printer_t error_printer) {
    return (parser_t){
        .lexer = lexer,
        .allocator = allocator,

        .item_head = NULL,
        .item_tail = NULL,

        .has_error = false,
        .error_printer = error_printer,
    };
}

#define LL_APPEND(head, tail, node) \
    if (*head == NULL) {            \
        *head = *tail = item;       \
    } else {                        \
        (*tail)->next = item;       \
        *tail = item;               \
    }

/**
 * Given the head and the tail of a statement's linked list, appends 'item'
 * to it.
 */
static void stmt_list_append(
    ast_stmt_node** head, ast_stmt_node** tail, ast_stmt_node* item
) {
    LL_APPEND(head, tail, item);
}

/**
 * Given the head and the tail of a linked list of items, appends 'item'
 * to it.
 */
static void item_list_append(
    ast_item_node** head, ast_item_node** tail, ast_item_node* item
) {
    LL_APPEND(head, tail, item);
}

/**
 * Given the head and the tail of a linked list of params, appends 'item'
 * to it.
 */
static void param_list_append(
    ast_param** head, ast_param** tail, ast_param* item
) {
    LL_APPEND(head, tail, item);
}

#undef LL_APPEND

static void insert_item(parser_t* parser, ast_item_node* node) {
    item_list_append(&parser->item_head, &parser->item_tail, node);
}

static token advance(parser_t* parser) {
    parser->prev = parser->curr;

    token_result res;
    while (!(res = lex_advance(&parser->lexer)).ok) {
        lex_e_print(&parser->lexer, res.e, parser->error_printer);
    }

    parser->curr = res.t;
    return res.t;
}

static inline bool is_eof(parser_t* parser) {
    return parser->curr.type == TOK_EOF;
}

static inline token peek(parser_t* parser) { return parser->curr; }

/*
 * Checks if current token's type is `tt`. Returns true and consumes the
 * token if that is the case.
 *
 * Current token is left intact if the token type does not match.
 * i.o.w, advance is not called.
 */
static bool match(parser_t* parser, token_type tt) {
    token tok = peek(parser);
    bool matches = tok.type == tt;

    if (matches) {
        advance(parser);
    }

    return matches;
}

/*
 * Like match, but die if the token is not of the expected type.
 * Unlike match, this returns the token that was consumed.
 */
static token expect(parser_t* parser, token_type tt, const char* fmt, ...) {
    if (!match(parser, tt)) {
        token offending =
            parser->curr.type == TOK_EOF ? parser->prev : parser->curr;

        parser->has_error = true;
        va_list args;
        va_start(args, fmt);
        parser->error_printer.error(parser->lexer.src, &offending.span,
                                    ERROR_KIND_SYNTAX, fmt, args);
        va_end(args);
        exit(1);
    }

    return parser->prev;
}

static vec_typename typename_tuple_items(parser_t* parser, bool function) {
    vec_typename items = vec_make(parser->allocator);
    while (!is_eof(parser) && !match(parser, TOK_PAREN_CLOSE)) {
        ast_typename* param = typename(parser);
        vec_push(&items, &param);

        if (!match(parser, TOK_COMMA)) {
            advance(parser);  // ')' handled below
            break;
        }
    }

    if (parser->prev.type != TOK_PAREN_CLOSE) {
        syntax_error_at_previous_and_die(
            parser,
            function ? "EOF while parsing function type params"
                     : "EOF while parsing tuple items"
        );
    }

    return items;
}

static ast_typename* function_typename(parser_t* parser) {
    expect(parser, TOK_PAREN_OPEN, "expected '(' after 'fn'");

    vec_typename params = typename_tuple_items(parser, true);

    ast_typename* return_type;
    if (match(parser, TOK_ARROW_RIGHT)) {
        return_type = typename(parser);
    } else {
        return_type = make_ast_typename_unit(parser->allocator);
    }

    return make_ast_typename_function(parser->allocator, params, return_type);
}

static ast_typename* typename_tuple(parser_t* parser) {
    vec_typename items = typename_tuple_items(parser, false);
    return make_ast_typename_tuple(parser->allocator, items);
}

static ast_typename* typename_integer(parser_t* parser) {
    token_type tt = parser->prev.type;

    bool is_signed = tt == TOK_I8 || tt == TOK_I16 || tt == TOK_I32;

    ast_integer_size size = INTEGER_SIZE_32;
    switch (parser->prev.type) {
        case TOK_I8:
        case TOK_U8:
            size = INTEGER_SIZE_8;
            break;

        case TOK_I16:
        case TOK_U16:
            size = INTEGER_SIZE_16;
            break;

        case TOK_I32:
        case TOK_U32:
            size = INTEGER_SIZE_32;
            break;
    }

    return make_ast_typename_integer(parser->allocator, is_signed, size);
}

static ast_typename* typename(parser_t* parser) {
    if (match(parser, TOK_KW_BOOLEAN)) {
        return make_ast_typename(parser->allocator, TYPE_NAME_BOOLEAN);
    } else if (match(parser, TOK_U8) || match(parser, TOK_I8) ||
               match(parser, TOK_U16) || match(parser, TOK_I16) ||
               match(parser, TOK_U32) || match(parser, TOK_I32)) {
        return typename_integer(parser);
    } else if (match(parser, TOK_KW_STRING)) {
        return make_ast_typename(parser->allocator, TYPE_NAME_STRING);
    } else if (match(parser, TOK_PAREN_OPEN)) {
        return typename_tuple(parser);
    } else if (match(parser, TOK_FN)) {
        return function_typename(parser);
    } else {
        syntax_error_at_current_and_die(parser, "expected a type");
    }
}

static ast_item_node* item(parser_t* parser) {
    if (match(parser, TOK_FN)) {
        return function_decl(parser);
    }

    syntax_error_at_current(parser, "expected function declaration");

    while (!is_eof(parser) && parser->curr.type != TOK_FN) {
        advance(parser);
    }

    return NULL;
}

static ast_param* params(parser_t* parser) {
    ast_param* head = NULL;
    ast_param* tail = NULL;

    while (!is_eof(parser) && peek(parser).type != TOK_PAREN_CLOSE) {
        token param_name =
            expect(parser, TOK_IDEN, "expected a parameter name");

        expect(parser, TOK_COLON, "expected ':' after parameter name");
        ast_typename* type = typename(parser);

        ast_param* item = make_ast_param(parser->allocator, param_name, type);
        param_list_append(&head, &tail, item);

        if (!match(parser, TOK_COMMA)) {
            break;
        }
    }

    return head;
}

static ast_item_node* function_decl(parser_t* parser) {
    // 'fn' token is consumed before calling function_decl

    token name = expect(parser, TOK_IDEN, "expected an identifier after 'fn'");

    expect(parser, TOK_PAREN_OPEN, "expected a '(' after function name");
    ast_param* params_list = params(parser);
    expect(parser, TOK_PAREN_CLOSE, "expected a ')' after function params");

    ast_typename* return_type;
    if (match(parser, TOK_ARROW_RIGHT)) {
        return_type = typename(parser);
    } else {
        return_type = make_ast_typename_unit(parser->allocator);
    }

    expect(parser, TOK_BRACE_OPEN, "expected function body");

    ast_stmt_node* body = NULL;
    ast_stmt_node* body_tail = NULL;

    while (!is_eof(parser) && !match(parser, TOK_BRACE_CLOSE)) {
        ast_stmt_node* curr = stmt(parser);
        stmt_list_append(&body, &body_tail, curr);
    }

    return make_ast_function(
        parser->allocator,
        name,
        params_list,
        body,
        return_type
    );
}

static ast_stmt_node* stmt(parser_t* parser) {
    ast_stmt_node* node = NULL;

    switch (peek(parser).type) {
        case TOK_LET:
            node = var_decl(parser);
            break;

        case TOK_BRACE_OPEN:
            node = block(parser);
            break;

        case TOK_IF:
            node = if_else(parser);
            break;

        case TOK_WHILE:
            node = while_(parser);
            break;

        default:
            node = expr_stmt(parser);
    }

    return node;
}

static ast_stmt_node* var_decl(parser_t* parser) {
    advance(parser);  // let

    bool mut = match(parser, TOK_MUT);

    token name = name = expect(
        parser,
        TOK_IDEN,
        mut ? "expected identifier after 'let mut'"
            : "expected identifier after 'let'"
    );

    ast_typename* type = NULL;
    if (match(parser, TOK_COLON)) {
        type = typename(parser);
    }

    ast_expr_node* value = match(parser, TOK_ASSIGN) ? expr(parser) : NULL;

    expect(parser, TOK_SEMI, "expected ';' after variable declaration");

    return make_ast_var_decl(parser->allocator, name, type, value, mut);
}

static ast_stmt_node* block(parser_t* parser) {
    advance(parser);  // {

    ast_stmt_node* body = NULL;
    ast_stmt_node* tail = NULL;

    while (peek(parser).type != TOK_BRACE_CLOSE && peek(parser).type != TOK_EOF
    ) {
        ast_stmt_node* next = stmt(parser);
        stmt_list_append(&body, &tail, next);
    }

    expect(parser, TOK_BRACE_CLOSE, "unclosed block");

    return make_ast_block(parser->allocator, body);
}

static ast_stmt_node* if_else(parser_t* parser) {
    advance(parser);  // if
    ast_expr_node* condition = expr(parser);

    token brace_open = peek(parser);
    if (brace_open.type != TOK_BRACE_OPEN) {
        syntax_error_at_current_and_die(parser, "expected '{' after if");
    }

    ast_stmt_node* body = block(parser);

    ast_stmt_node* else_body = NULL;

    if (match(parser, TOK_ELSE)) {
        if (peek(parser).type == TOK_IF) {
            else_body = if_else(parser);
        } else if (peek(parser).type == TOK_BRACE_OPEN) {
            else_body = block(parser);
        } else {
            syntax_error_at_current_and_die(parser, "expected 'if' or '{' after else");
        }
    }

    return make_ast_if_else(parser->allocator, condition, body, else_body);
}

static ast_stmt_node* while_(parser_t* parser) {
    advance(parser);  // while

    ast_expr_node* condition = expr(parser);

    token brace_open = peek(parser);
    if (brace_open.type != TOK_BRACE_OPEN) {
        syntax_error_at_current_and_die(parser, "expected '{' after while");
    }

    ast_stmt_node* body = block(parser);

    return make_ast_while(parser->allocator, condition, body);
}

static ast_stmt_node* expr_stmt(parser_t* parser) {
    ast_expr_node* expr_node = expr(parser);
    ast_stmt_node* node = make_ast_expr_stmt(parser->allocator, expr_node);

    expect(parser, TOK_SEMI, "expected ';' after statement");

    return node;
}

static ast_expr_node* expr(parser_t* parser) { return assign(parser); }

static ast_expr_node* assign(parser_t* parser) {
    ast_expr_node* left = or_op(parser);

    while (!is_eof(parser) && match(parser, TOK_ASSIGN)) {
        if (left->type != AST_IDEN) {
            syntax_error_at_current_and_die(parser, "can only assign to identifiers");
        }

        ast_expr_node* right = assign(parser);

        left = make_ast_binary(parser->allocator, TOK_ASSIGN, left, right);
    }

    return left;
}

static ast_expr_node* or_op(parser_t* parser) {
    ast_expr_node* left = and_op(parser);

    while (!is_eof(parser) && match(parser, TOK_OR)) {
        ast_expr_node* right = and_op(parser);
        left = make_ast_binary(parser->allocator, TOK_OR, left, right);
    }

    return left;
}

static ast_expr_node* and_op(parser_t* parser) {
    ast_expr_node* left = bitwise_or(parser);

    while (!is_eof(parser) && match(parser, TOK_AND)) {
        ast_expr_node* right = bitwise_or(parser);
        left = make_ast_binary(parser->allocator, TOK_AND, left, right);
    }

    return left;
}

static ast_expr_node* bitwise_or(parser_t* parser) {
    ast_expr_node* left = bitwise_xor(parser);

    while (!is_eof(parser) && match(parser, TOK_PIPE)) {
        ast_expr_node* right = bitwise_xor(parser);
        left = make_ast_binary(parser->allocator, TOK_PIPE, left, right);
    }

    return left;
}

static ast_expr_node* bitwise_xor(parser_t* parser) {
    ast_expr_node* left = bitwise_and(parser);

    while (!is_eof(parser) && match(parser, TOK_CARET)) {
        ast_expr_node* right = bitwise_and(parser);
        left = make_ast_binary(parser->allocator, TOK_CARET, left, right);
    }

    return left;
}

static ast_expr_node* bitwise_and(parser_t* parser) {
    ast_expr_node* left = equality(parser);

    while (!is_eof(parser) && match(parser, TOK_AMP)) {
        ast_expr_node* right = equality(parser);
        left = make_ast_binary(parser->allocator, TOK_AMP, left, right);
    }

    return left;
}

static ast_expr_node* equality(parser_t* parser) {
    ast_expr_node* left = comparision(parser);

    while (!is_eof(parser) && (match(parser, TOK_EQ) || match(parser, TOK_NEQ))
    ) {
        token op = parser->prev;

        ast_expr_node* right = comparision(parser);
        left = make_ast_binary(parser->allocator, op.type, left, right);
    }

    return left;
}

static ast_expr_node* comparision(parser_t* parser) {
    ast_expr_node* left = term(parser);

    while (!is_eof(parser) &&
           (match(parser, TOK_GT) || match(parser, TOK_GTEQ) ||
            match(parser, TOK_LT) || match(parser, TOK_LTEQ))) {
        token op = parser->prev;

        ast_expr_node* right = term(parser);
        left = make_ast_binary(parser->allocator, op.type, left, right);
    }

    return left;
}

static ast_expr_node* term(parser_t* parser) {
    ast_expr_node* left = factor(parser);

    while (!is_eof(parser) &&
           (match(parser, TOK_PLUS) || match(parser, TOK_MINUS))) {
        token op = parser->prev;

        ast_expr_node* right = factor(parser);
        left = make_ast_binary(parser->allocator, op.type, left, right);
    }

    return left;
}

static ast_expr_node* factor(parser_t* parser) {
    ast_expr_node* left = unary(parser);

    while (!is_eof(parser) &&
           (match(parser, TOK_MUL) || match(parser, TOK_DIV) ||
            match(parser, TOK_PERC))) {
        token op = parser->prev;

        ast_expr_node* right = unary(parser);
        left = make_ast_binary(parser->allocator, op.type, left, right);
    }

    return left;
}

static ast_expr_node* unary(parser_t* parser) {
    if (match(parser, TOK_MINUS) || match(parser, TOK_PLUS) ||
        match(parser, TOK_BANG)) {
        token_type tt = parser->prev.type;
        ast_expr_node* expr = unary(parser);
        return make_ast_unary(parser->allocator, tt, expr);
    }

    return function_call(parser);
}

static vec_expr arguments(parser_t* parser) {
    vec_expr args = vec_make(parser->allocator);

    while (!is_eof(parser) && peek(parser).type != TOK_PAREN_CLOSE) {
        ast_expr_node* arg = expr(parser);
        vec_push(&args, &arg);

        if (!match(parser, TOK_COMMA)) {
            break;
        }
    }

    expect(parser, TOK_PAREN_CLOSE, "expected a ')' after function arguments");

    return args;
}

static ast_expr_node* function_call(parser_t* parser) {
    ast_expr_node* maybe_callee = primary(parser);

    while (match(parser, TOK_PAREN_OPEN)) {
        maybe_callee =
            make_ast_call(parser->allocator, maybe_callee, arguments(parser));
    }

    return maybe_callee;
}

static ast_expr_node* primary(parser_t* parser) {
    token_type tt = peek(parser).type;

    switch (tt) {
        case TOK_NUM:
            return number(parser);

        case TOK_IDEN:
            return iden(parser);

        case TOK_PAREN_OPEN:
            return group(parser);

        case TOK_STR:
            return str(parser);

        case TOK_TRUE:
        case TOK_FALSE:
            return boolean(parser);

        case TOK_FN:
            return lambda(parser);

        default:
            syntax_error_at_current_and_die(parser, "expected primary expression");
    }
}

static ast_expr_node* number(parser_t* parser) {
    token tok = peek(parser);
    advance(parser);
    return make_ast_num(parser->allocator, strtold(tok.span.span, NULL));
}

static ast_expr_node* iden(parser_t* parser) {
    token tok = peek(parser);
    advance(parser);
    return make_ast_identifier(parser->allocator, tok.span.span, tok.span.span_size);
}

static ast_expr_node* group(parser_t* parser) {
    advance(parser);  // (

    ast_expr_node* result = expr(parser);

    expect(parser, TOK_PAREN_CLOSE, "expected ')'");

    return result;
}

static ast_expr_node* str(parser_t* parser) {
#define SUBSTITUTE(chr, replacement) \
    case chr:                        \
        str[len++] = replacement;    \
        break;

    token tok = peek(parser);
    size_t size = tok.span.span_size - 1;

    char* str = ALLOC_ARRAY(parser->allocator, char, size);
    size_t len = 0;
    size_t i = 1;

    while (i < tok.span.span_size - 1) {
        if (tok.span.span[i] != '\\') {
            str[len++] = tok.span.span[i++];
            continue;
        }

        i++;

        if (isdigit(tok.span.span[i])) {
            char* end;
            char c = (char)strtol(tok.span.span + i, &end, 10);
            i = end - tok.span.span;

            str[len++] = c;
            continue;
        }

        if (tok.span.span[i] == 'x') {
            i++;  // x

            char* end;
            char c = (char)strtol(tok.span.span + i, &end, 16);
            i = end - tok.span.span;

            str[len++] = c;
            continue;
        }

        switch (tok.span.span[i++]) {
            SUBSTITUTE('\\', '\\')
            SUBSTITUTE('"', '"')
            SUBSTITUTE('a', '\a')
            SUBSTITUTE('b', '\b')
            SUBSTITUTE('n', '\n')
            SUBSTITUTE('r', '\r')
            SUBSTITUTE('t', '\t')

            default:
                syntax_error_at_current_and_die(parser, "invalid escape sequence");
        }
    }

    str[len] = '\0';

    advance(parser);
    return make_ast_str(parser->allocator, str, len, size);

#undef SUBSTITUTE
}

static ast_expr_node* boolean(parser_t* parser) {
    advance(parser);
    return make_ast_bool(parser->allocator, parser->prev.type == TOK_TRUE);
}

static ast_expr_node* lambda(parser_t* parser) {
    advance(parser);  // fn

    expect(parser, TOK_PAREN_OPEN, "expected a '(' after function name");
    ast_param* params_list = params(parser);
    expect(parser, TOK_PAREN_CLOSE, "expected a ')' after function params");

    ast_typename* return_type;
    if (match(parser, TOK_ARROW_RIGHT)) {
        return_type = typename(parser);
    } else {
        return_type = make_ast_typename_unit(parser->allocator);
    }

    expect(parser, TOK_BRACE_OPEN, "expected function body");

    ast_stmt_node* body = NULL;
    ast_stmt_node* body_tail = NULL;

    while (!is_eof(parser) && !match(parser, TOK_BRACE_CLOSE)) {
        ast_stmt_node* curr = stmt(parser);
        stmt_list_append(&body, &body_tail, curr);
    }

    return make_ast_lambda(parser->allocator, params_list, body, return_type);
}

static bool validate_curly_brace_balance(error_printer_t error_printer, lexer_t lexer) {
    bool ret = true;

    token_result curr;

    allocator_t* allocator = gpa();

    /* A list of '{' tokens we haven't see a closing '}' for. */
    token* opened = ALLOC_ARRAY(allocator, token, 8);
    size_t opened_cap = 8;
    size_t opened_len = 0;

    for (;;) {
        curr = lex_advance(&lexer);

        if (!curr.ok) {
            continue;
        }

        if (curr.t.type == TOK_EOF) {
            break;
        }

        if (curr.t.type == TOK_BRACE_OPEN) {
            if (opened_cap < opened_len + 1) {
                size_t new_cap = opened_cap * 2;
                opened = RESIZE_ARRAY(allocator, opened, token, opened_cap, new_cap);

                opened_cap = new_cap;
            }

            opened[opened_len++] = curr.t;
        } else if (curr.t.type == TOK_BRACE_CLOSE) {
            if (opened_len > 0) {
                opened_len--;
                continue;
            }

            /* This '}' is a stray. Report it. */
            syntax_error_at_span(
                error_printer,
                lexer.src,
                &curr.t.span,
                "unexpected closing delimiter '}'"
            );

            ret = false;
        }
    }

    if (opened_len > 0) {
        ret = false;

        for (size_t i = 0; i < opened_len; i++) {
            syntax_error_at_span(
                error_printer,
                lexer.src,
                &opened[i].span,
                "unclosed delimiter '{'"
            );
        }
    }

    FREE_ARRAY(allocator, opened, token, opened_cap);
    return ret;
}

bool parse_with_error_printer(
    allocator_t* allocator, char* src, size_t src_len,
    error_printer_t error_printer,
    ast_item_node** out) {

    parser_t parser = make_parser(allocator, make_lexer(src, src_len),
                                  error_printer);
    advance(&parser);

    /* Pass 1: Check if the braces are balanced. */

    if (!validate_curly_brace_balance(error_printer, make_lexer(src, src_len))) {
        /*
         * Regardless of success status, we must write the parsed AST to
         * \p{out}, which is empty in this case.
         */
        *out = NULL;

        return false;
    }

    /* Pass 2: Parse into AST */
    while (!is_eof(&parser)) {
        ast_item_node* curr = item(&parser);

        /*
         * item() may return NULL if there was an error while parsing
         * the item.
         */
        if (curr != NULL) {
            insert_item(&parser, curr);
        }
    }

    *out = parser.item_head;

    return !parser.has_error;
}

bool parse(
    allocator_t* allocator, char* src, size_t src_len, ast_item_node** out
) {
    return parse_with_error_printer(
        allocator, src, src_len,
        default_error_printer, out);
}
