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
} parser_t;

static ast_node* expr(parser_t* parser);
static ast_node* term(parser_t* parser);
static ast_node* factor(parser_t* parser);
static ast_node* number(parser_t* parser);
static ast_node* iden(parser_t* parser);
static ast_node* group(parser_t* parser);
static ast_node* str(parser_t* parser);

static void __die(char* err) {
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
    };
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

static ast_node* expr(parser_t* parser) {
    ast_node* left = term(parser);

    while (!lex_eof(&parser->lexer) &&
           (parser->curr.type == TOK_PLUS || parser->curr.type == TOK_MINUS)) {
        token op = parser->curr;
        advance(parser);

        ast_node* right = term(parser);
        left = make_ast_binary(parser->allocator, op.type, left, right);
    }

    return left;
}

static ast_node* term(parser_t* parser) {
    ast_node* left = factor(parser);

    while (!lex_eof(&parser->lexer) &&
           (parser->curr.type == TOK_MUL || parser->curr.type == TOK_DIV)) {
        token op = parser->curr;
        advance(parser);

        ast_node* right = factor(parser);
        left = make_ast_binary(parser->allocator, op.type, left, right);
    }

    return left;
}

static ast_node* factor(parser_t* parser) {
    token_type tt = parser->curr.type;

    switch (tt) {
        case TOK_NUM:
            return number(parser);

        case TOK_IDEN:
            return iden(parser);

        case TOK_PAREN_OPEN:
            return group(parser);

        case TOK_STR:
            return str(parser);

        default:
            __die("expected primary expression");
    }
}

static ast_node* number(parser_t* parser) {
    token tok = parser->curr;
    advance(parser);
    return make_ast_num(parser->allocator, strtold(tok.span, NULL));
}

static ast_node* iden(parser_t* parser) {
    token tok = parser->curr;
    advance(parser);
    return make_ast_identifier(parser->allocator, tok.span, tok.span_size);
}

static ast_node* group(parser_t* parser) {
    advance(parser);  // (

    ast_node* result = expr(parser);
    token closing = parser->curr;

    if (closing.type != TOK_PAREN_CLOSE) {
        __die("expected ')'");
    }

    advance(parser);  // )

    return result;
}

static ast_node* str(parser_t* parser) {
#define SUBSTITUTE(chr, replacement) \
    case chr:                        \
        str[len++] = replacement;    \
        break;

    token tok = parser->curr;
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

ast_node* parse(allocator_t* allocator, char* src, size_t src_len) {
    parser_t parser = make_parser(allocator, make_lexer(src, src_len));
    advance(&parser);
    return expr(&parser);
}
