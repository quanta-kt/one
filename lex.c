#include "lex.h"

#include <ctype.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
    token_type tt;
    char keyword[8];
    size_t keyword_len;
} keyword;

#define TABLE_ENTRY(_tt, _kw) \
    {.keyword = _kw, .keyword_len = sizeof(_kw) - 1, .tt = _tt}

keyword keyword_table[] = {
    TABLE_ENTRY(TOK_LET, "let"),
    TABLE_ENTRY(TOK_MUT, "mut"),
    TABLE_ENTRY(TOK_FALSE, "false"),
    TABLE_ENTRY(TOK_TRUE, "true"),
    TABLE_ENTRY(TOK_IF, "if"),
    TABLE_ENTRY(TOK_ELSE, "else"),
    TABLE_ENTRY(TOK_WHILE, "while"),
    TABLE_ENTRY(TOK_FOR, "for"),
    TABLE_ENTRY(TOK_FN, "fn"),
    TABLE_ENTRY(TOK_STRUCT, "struct"),
    TABLE_ENTRY(TOK_IMPL, "impl"),
    TABLE_ENTRY(TOK_RETURN, "return"),
    TABLE_ENTRY(TOK_LET, "async"),
    TABLE_ENTRY(TOK_LET, "await"),
    TABLE_ENTRY(TOK_U8, "u8"),
    TABLE_ENTRY(TOK_U16, "u16"),
    TABLE_ENTRY(TOK_U32, "u32"),
    TABLE_ENTRY(TOK_I8, "i8"),
    TABLE_ENTRY(TOK_I16, "i16"),
    TABLE_ENTRY(TOK_I32, "i32"),
    TABLE_ENTRY(TOK_KW_STRING, "string"),
    TABLE_ENTRY(TOK_KW_BOOLEAN, "boolean"),
};

const size_t keyword_table_len =
    sizeof(keyword_table) / sizeof(keyword_table[0]);

#undef TABLE_ENTRY

static token make_token(token_type tt, lexer_t* lex) {
    token tok = (token){
        .type = tt,
        .span = lex->start,
        .span_size = lex->curr - lex->start,
        .line = lex->start_line,
        .col = lex->start_col,
    };

    lex->start = lex->curr;
    lex->start_col = lex->col;
    lex->start_line = lex->line;
    return tok;
}

static lex_error make_lex_error(lex_error_type type, char* span, size_t size) {
    return (lex_error){.type = type, .span = span, .span_size = size};
}

static token_result token_ok(token t) {
    return (token_result){
        .ok = true,
        .t = t,
    };
}

static token_result token_err(lex_error e) {
    return (token_result){
        .ok = false,
        .e = e,
    };
}

static bool is_digit(char c) { return c >= '0' && c <= '9'; }

static bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool is_alpha_num(char c) { return is_alpha(c) || is_digit(c); }

static char peek(lexer_t* lex) { return *lex->curr; }
static bool lex_eof(lexer_t* lex) { return peek(lex) == '\0'; }
static char advance(lexer_t* lex) {
    if (lex_eof(lex)) {
        return '\0';
    }

    if (*lex->curr == '\n') {
        lex->line++;
        lex->col = 0;  // gets corrected to 1 below with 'col++'
    }

    lex->col++;

    return *lex->curr++;
}

static token token_num(lexer_t* lex) {
    bool seen_decimal_point = false;

    while (is_digit(peek(lex)) || (!seen_decimal_point && peek(lex) == '.')) {
        seen_decimal_point = seen_decimal_point || peek(lex) == '.';
        advance(lex);
    }

    return make_token(TOK_NUM, lex);
}

static token token_iden(lexer_t* lex) {
    while (is_alpha_num(peek(lex))) advance(lex);
    size_t len = lex->curr - lex->start;

    token_type tt = TOK_IDEN;

    for (size_t i = 0; i < keyword_table_len; i++) {
        keyword kw = keyword_table[i];

        if (kw.keyword_len != len) continue;

        if (memcmp(lex->start, kw.keyword, kw.keyword_len) == 0) {
            tt = kw.tt;
            break;
        }
    }

    return make_token(tt, lex);
}

static token_result token_str(lexer_t* lex) {
    while (!lex_eof(lex) && peek(lex) != '"') {
        advance(lex);
    }

    if (peek(lex) != '"') {
        return token_err(make_lex_error(
            LEX_ERR_UNTERMINATED_STRING,
            lex->start,
            lex->curr - lex->start
        ));
    }

    advance(lex);  // "

    return token_ok(make_token(TOK_STR, lex));
}

static void skip_whitespace(lexer_t* lex) {
    while (!lex_eof(lex) && isspace(peek(lex))) {
        advance(lex);
    }
}

lexer_t make_lexer(char* src, size_t size) {
    return (lexer_t){
        .src = src,
        .curr = src,
        .size = size,
        .line = 1,
        .col = 1,

        .start = src,
        .start_line = 1,
        .start_col = 1,
    };
}

static token_result make_token_single_char(lexer_t* lex, token_type tt) {
    size_t col = lex->col;
    return token_ok(make_token(tt, lex));
}

token_result lex_advance(lexer_t* lex) {
    skip_whitespace(lex);

    lex->start_line = lex->line;
    lex->start_col = lex->col;
    lex->start = lex->curr;

    if (lex_eof(lex)) {
        return token_ok(make_token(TOK_EOF, lex));
    }

    char c = advance(lex);

    if (is_alpha(c)) {
        return token_ok(token_iden(lex));
    }

    if (is_digit(c)) {
        return token_ok(token_num(lex));
    }

    switch (c) {
        case '+':
            return make_token_single_char(lex, TOK_PLUS);
        case '-': {
            if (peek(lex) == '>') {
                advance(lex);
                return token_ok(make_token(TOK_ARROW_RIGHT, lex));
            }

            return make_token_single_char(lex, TOK_MINUS);
        }
        case '/':
            return make_token_single_char(lex, TOK_DIV);
        case '*':
            return make_token_single_char(lex, TOK_MUL);
        case '%':
            return make_token_single_char(lex, TOK_PERC);
        case '(':
            return make_token_single_char(lex, TOK_PAREN_OPEN);
        case ')':
            return make_token_single_char(lex, TOK_PAREN_CLOSE);
        case '{':
            return make_token_single_char(lex, TOK_BRACE_OPEN);
        case '}':
            return make_token_single_char(lex, TOK_BRACE_CLOSE);
        case ':':
            return make_token_single_char(lex, TOK_COLON);
        case ',':
            return make_token_single_char(lex, TOK_COMMA);
        case ';':
            return make_token_single_char(lex, TOK_SEMI);
        case '^':
            return make_token_single_char(lex, TOK_CARET);

        case '=': {
            if (peek(lex) == '=') {
                advance(lex);
                return token_ok(make_token(TOK_EQ, lex));
            }
            return make_token_single_char(lex, TOK_ASSIGN);
        }
        case '!': {
            if (peek(lex) == '=') {
                advance(lex);
                return token_ok(make_token(TOK_NEQ, lex));
            }
            return make_token_single_char(lex, TOK_BANG);
        }

        case '|': {
            if (peek(lex) == '|') {
                advance(lex);
                return token_ok(make_token(TOK_OR, lex));
            }

            return make_token_single_char(lex, TOK_PIPE);
        }
        case '&': {
            if (peek(lex) == '&') {
                advance(lex);
                return token_ok(make_token(TOK_AND, lex));
            }

            return make_token_single_char(lex, TOK_AMP);
        }

        case '>': {
            if (peek(lex) == '=') {
                advance(lex);
                return token_ok(make_token(TOK_GTEQ, lex));
            }

            return make_token_single_char(lex, TOK_GT);
        }
        case '<': {
            if (peek(lex) == '=') {
                advance(lex);
                return token_ok(make_token(TOK_LTEQ, lex));
            }

            return make_token_single_char(lex, TOK_LT);
        }

        case '"':
            return token_str(lex);
    }

    token_result ret =
        token_err(make_lex_error(LEX_ERR_UNEXPECTED_CHAR, lex->curr, 1));

    advance(lex);

    return ret;
}
