#ifndef LEX_H
#define LEX_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    TOK_IDEN,

    TOK_NUM,
    TOK_STR,

    TOK_TRUE,
    TOK_FALSE,

    TOK_PLUS,
    TOK_MINUS,
    TOK_DIV,
    TOK_MUL,
    TOK_PERC,
    TOK_GT,
    TOK_GTEQ,
    TOK_LT,
    TOK_LTEQ,
    TOK_PIPE,
    TOK_AMP,
    TOK_OR,
    TOK_AND,
    TOK_CARET,

    TOK_BANG,  // !
    TOK_EQ,    // ==
    TOK_NEQ,   // !=

    TOK_LET,
    TOK_MUT,
    TOK_ASSIGN,

    TOK_IF,
    TOK_ELSE,
    TOK_WHILE,
    TOK_FOR,

    TOK_FN,
    TOK_STRUCT,
    TOK_IMPL,

    TOK_RETURN,

    TOK_ASYNC,
    TOK_AWAIT,

    TOK_PAREN_OPEN,
    TOK_PAREN_CLOSE,

    TOK_BRACE_OPEN,
    TOK_BRACE_CLOSE,

    TOK_COLON,

    TOK_SEMI,

    TOK_EOF,
} token_type;

typedef struct {
    token_type type;
    char* span;
    size_t span_size;
} token;

typedef enum {
    LEX_ERR_UNEXPECTED_CHAR,
    LEX_ERR_UNTERMINATED_STRING,
} lex_error_type;

typedef struct {
    lex_error_type type;
    char* span;
    size_t span_size;
} lex_error;

typedef struct {
    union {
        token t;
        lex_error e;
    };

    bool ok : 1;
} token_result;

typedef struct {
    char* src;
    size_t size;

    char* curr;
} lexer_t;

lexer_t make_lexer(char* src, size_t size);
bool lex_eof(lexer_t* lex);
token_result lex_advance();

#endif  // LEX_H
