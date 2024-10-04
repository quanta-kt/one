#include "parser.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "ast.h"

typedef struct {
    lexer_t lexer;
    allocator_t* allocator;

    token curr;
    token prev;

    ast_stmt_node* ast_head;
    ast_stmt_node* ast_tail;
} parser_t;

static ast_stmt_node* var_decl(parser_t* parser);
static ast_stmt_node* block(parser_t* parser);
static ast_stmt_node* if_else(parser_t* parser);
static ast_stmt_node* expr_stmt(parser_t* parser);

static ast_expr_node* expr(parser_t* parser);
static ast_expr_node* or_op(parser_t* parser);
static ast_expr_node* and_op(parser_t* parser);
static ast_expr_node* bitwise_or(parser_t* parser);
static ast_expr_node* bitwise_xor(parser_t* parser);
;
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

static void __die(char const* err) {
    fprintf(stderr, "%s\n", err);
    exit(1);
}

static void lex_e_print(lex_error e) {
    switch (e.type) {
        case LEX_ERR_UNEXPECTED_CHAR: {
            fprintf(stderr, "Unexpected character: '%c'", *e.span);
            break;
        }

        case LEX_ERR_UNTERMINATED_STRING: {
            fprintf(
                stderr,
                "Unterminated string: %.*s",
                (int)e.span_size,
                e.span
            );
            break;
        }
    }

    exit(1);
}

static parser_t make_parser(allocator_t* allocator, lexer_t lexer) {
    return (parser_t){
        .lexer = lexer,
        .allocator = allocator,

        .ast_head = NULL,
        .ast_tail = NULL,
    };
}

static void insert_stmt(parser_t* parser, ast_stmt_node* node) {
    if (parser->ast_head == NULL) {
        parser->ast_head = (parser->ast_tail = node);
        return;
    }

    parser->ast_tail->next = node;
    parser->ast_tail = node;
}

static token advance(parser_t* parser) {
    parser->prev = parser->curr;

    token_result res;
    while (!(res = lex_advance(parser->lexer)).ok) {
        lex_e_print(res.e);
    }

    parser->curr = res.t;
    return res.t;
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
static token expect(parser_t* parser, token_type tt, char const* message) {
    if (!match(parser, tt)) {
        __die(message);
    }

    return parser->prev;
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

    ast_expr_node* value = match(parser, TOK_ASSIGN) ? expr(parser) : NULL;

    expect(parser, TOK_SEMI, "expected ';' after variable declaration");

    return make_ast_var_decl(parser->allocator, name, value, mut);
}

static ast_stmt_node* block(parser_t* parser) {
    advance(parser);  // {

    ast_stmt_node* body = NULL;
    ast_stmt_node* tail = NULL;

    while (peek(parser).type != TOK_BRACE_CLOSE && peek(parser).type != TOK_EOF
    ) {
        ast_stmt_node* next = stmt(parser);

        if (tail == NULL) {
            tail = next;
            body = next;
        } else {
            tail->next = next;
        }

        tail = next;
    }

    expect(parser, TOK_BRACE_CLOSE, "unclosed block");

    return make_ast_block(parser->allocator, body);
}

static ast_stmt_node* if_else(parser_t* parser) {
    advance(parser);  // if
    ast_expr_node* condition = expr(parser);

    token brace_open = peek(parser);
    if (brace_open.type != TOK_BRACE_OPEN) {
        __die("expected '{' after if");
    }

    ast_stmt_node* body = block(parser);

    ast_stmt_node* else_body = NULL;

    if (match(parser, TOK_ELSE)) {
        if (peek(parser).type == TOK_IF) {
            else_body = if_else(parser);
        } else if (peek(parser).type == TOK_BRACE_OPEN) {
            else_body = block(parser);
        } else {
            __die("expected 'if' or '{' after else");
        }
    }

    return make_ast_if_else(parser->allocator, condition, body, else_body);
}

static ast_stmt_node* expr_stmt(parser_t* parser) {
    ast_expr_node* expr_node = expr(parser);
    ast_stmt_node* node = make_ast_expr_stmt(parser->allocator, expr_node);

    expect(parser, TOK_SEMI, "expected ';' after statement");

    return node;
}

static ast_expr_node* expr(parser_t* parser) { return or_op(parser); }

static ast_expr_node* or_op(parser_t* parser) {
    ast_expr_node* left = and_op(parser);

    while (!lex_eof(&parser->lexer) && match(parser, TOK_OR)) {
        ast_expr_node* right = and_op(parser);
        left = make_ast_binary(parser->allocator, TOK_OR, left, right);
    }

    return left;
}

static ast_expr_node* and_op(parser_t* parser) {
    ast_expr_node* left = bitwise_or(parser);

    while (!lex_eof(&parser->lexer) && match(parser, TOK_AND)) {
        ast_expr_node* right = bitwise_or(parser);
        left = make_ast_binary(parser->allocator, TOK_AND, left, right);
    }

    return left;
}

static ast_expr_node* bitwise_or(parser_t* parser) {
    ast_expr_node* left = bitwise_xor(parser);

    while (!lex_eof(&parser->lexer) && match(parser, TOK_PIPE)) {
        ast_expr_node* right = bitwise_xor(parser);
        left = make_ast_binary(parser->allocator, TOK_PIPE, left, right);
    }

    return left;
}

static ast_expr_node* bitwise_xor(parser_t* parser) {
    ast_expr_node* left = bitwise_and(parser);

    while (!lex_eof(&parser->lexer) && match(parser, TOK_CARET)) {
        ast_expr_node* right = bitwise_and(parser);
        left = make_ast_binary(parser->allocator, TOK_CARET, left, right);
    }

    return left;
}

static ast_expr_node* bitwise_and(parser_t* parser) {
    ast_expr_node* left = equality(parser);

    while (!lex_eof(&parser->lexer) && match(parser, TOK_AMP)) {
        ast_expr_node* right = equality(parser);
        left = make_ast_binary(parser->allocator, TOK_AMP, left, right);
    }

    return left;
}

static ast_expr_node* equality(parser_t* parser) {
    ast_expr_node* left = comparision(parser);

    while (!lex_eof(&parser->lexer) &&
           (match(parser, TOK_EQ) || match(parser, TOK_NEQ))) {
        token op = parser->prev;

        ast_expr_node* right = comparision(parser);
        left = make_ast_binary(parser->allocator, op.type, left, right);
    }

    return left;
}

static ast_expr_node* comparision(parser_t* parser) {
    ast_expr_node* left = term(parser);

    while (!lex_eof(&parser->lexer) &&
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

    while (!lex_eof(&parser->lexer) &&
           (match(parser, TOK_PLUS) || match(parser, TOK_MINUS))) {
        token op = parser->prev;

        ast_expr_node* right = factor(parser);
        left = make_ast_binary(parser->allocator, op.type, left, right);
    }

    return left;
}

static ast_expr_node* factor(parser_t* parser) {
    ast_expr_node* left = unary(parser);

    while (!lex_eof(&parser->lexer) &&
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

static vec arguments(parser_t* parser) {
    vec args = make_vec(ast_expr_node*, parser->allocator);

    for (;;) {
        if (peek(parser).type == TOK_PAREN_CLOSE) {
            break;
        }

        ast_expr_node* arg = expr(parser);
        vec_push(&args, ast_expr_node*, &arg);

        match(parser, TOK_COMMA);
    }

    if (!match(parser, TOK_PAREN_CLOSE)) {
        vec_free(&args);
        __die("expected ')'");
    }

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

        default:
            __die("expected primary expression");
    }
}

static ast_expr_node* number(parser_t* parser) {
    token tok = peek(parser);
    advance(parser);
    return make_ast_num(parser->allocator, strtold(tok.span, NULL));
}

static ast_expr_node* iden(parser_t* parser) {
    token tok = peek(parser);
    advance(parser);
    return make_ast_identifier(parser->allocator, tok.span, tok.span_size);
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
    size_t size = tok.span_size - 1;

    char* str = ALLOC_ARRAY(parser->allocator, char, size);
    size_t len = 0;
    size_t i = 1;

    while (i < tok.span_size - 1) {
        if (tok.span[i] != '\\') {
            str[len++] = tok.span[i++];
            continue;
        }

        i++;

        if (isdigit(tok.span[i])) {
            char* end;
            char c = (char)strtol(tok.span + i, &end, 10);
            i = end - tok.span;

            str[len++] = c;
            continue;
        }

        if (tok.span[i] == 'x') {
            i++;  // x

            char* end;
            char c = (char)strtol(tok.span + i, &end, 16);
            i = end - tok.span;

            str[len++] = c;
            continue;
        }

        switch (tok.span[i]) {
            SUBSTITUTE('\\', '\\')
            SUBSTITUTE('a', '\a')
            SUBSTITUTE('b', '\b')
            SUBSTITUTE('n', '\n')
            SUBSTITUTE('r', '\r')
            SUBSTITUTE('t', '\t')

            default:
                __die("invalid escape sequence");
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

ast_stmt_node* parse(allocator_t* allocator, char* src, size_t src_len) {
    parser_t parser = make_parser(allocator, make_lexer(src, src_len));
    advance(&parser);

    do {
        insert_stmt(&parser, stmt(&parser));
    } while (!lex_eof(&parser.lexer));

    return parser.ast_head;
}
