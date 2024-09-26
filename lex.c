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
};

const size_t keyword_table_len =
    sizeof(keyword_table) / sizeof(keyword_table[0]);

#undef TABLE_ENTRY

static token make_token(token_type tt, char* span, size_t size) {
    return (token){.type = tt, .span = span, .span_size = size};
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

static token token_num(lexer_t* lex) {
    char* start = lex->curr;
    char* end = start;

    while (is_digit(*end) || *end == '.') end++;
    lex->curr = end;

    return make_token(TOK_NUM, start, end - start);
}

static token token_iden(lexer_t* lex) {
    char* start = lex->curr;
    char* end = start;

    while (is_alpha_num(*end)) end++;
    lex->curr = end;
    size_t len = end - start;

    token_type tt = TOK_IDEN;

    for (size_t i = 0; i < keyword_table_len; i++) {
        keyword kw = keyword_table[i];

        if (kw.keyword_len != len) continue;

        if (memcmp(start, kw.keyword, kw.keyword_len) == 0) {
            tt = kw.tt;
            break;
        }
    }

    return make_token(tt, start, len);
}

static token_result token_str(lexer_t* lex) {
    char* start = lex->curr;

    lex->curr++;  // "

    for (; !lex_eof(lex) && *lex->curr != '"'; lex->curr++);

    if (*lex->curr != '"') {
        return token_err(make_lex_error(
            LEX_ERR_UNTERMINATED_STRING,
            start,
            lex->curr - start
        ));
    }

    lex->curr++;  // "

    return token_ok(make_token(TOK_STR, start, lex->curr - start));
}

static void skip_whitespace(lexer_t* lex) {
    while (!lex_eof(lex) && isspace(*lex->curr)) lex->curr++;
}

lexer_t make_lexer(char* src, size_t size) {
    return (lexer_t){
        .src = src,
        .curr = src,
        .size = size,
    };
}

bool lex_eof(lexer_t* lex) { return *lex->curr == '\0'; }

token_result lex_advance(lexer_t* lex) {
#define SINGLE(tt) (token_ok(make_token(tt, lex->curr++, 1)))
    skip_whitespace(lex);

    if (lex_eof(lex)) {
        return token_ok(make_token(TOK_EOF, NULL, 0));
    }

    char c = *lex->curr;

    if (is_alpha(c)) {
        return token_ok(token_iden(lex));
    }

    if (is_digit(c)) {
        return token_ok(token_num(lex));
    }

    switch (c) {
        case '+':
            return SINGLE(TOK_PLUS);
        case '-':
            return SINGLE(TOK_MINUS);
        case '/':
            return SINGLE(TOK_DIV);
        case '*':
            return SINGLE(TOK_MUL);
        case '(':
            return SINGLE(TOK_PAREN_OPEN);
        case ')':
            return SINGLE(TOK_PAREN_CLOSE);
        case '{':
            return SINGLE(TOK_BRACE_OPEN);
        case '}':
            return SINGLE(TOK_BRACE_CLOSE);
        case ':':
            return SINGLE(TOK_COLON);
        case ';':
            return SINGLE(TOK_SEMI);
        case '=':
            return SINGLE(TOK_ASSIGN);
        case '>':
            return SINGLE(TOK_GT);
        case '<':
            return SINGLE(TOK_LT);
        case '"':
            return token_str(lex);
    }

    return token_err(make_lex_error(LEX_ERR_UNEXPECTED_CHAR, lex->curr++, 1));

#undef SINGLE
}
