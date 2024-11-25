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
    TOK_COMMA,
    TOK_SEMI,

    TOK_ARROW_RIGHT,  // ->

    TOK_U8,
    TOK_U16,
    TOK_U32,
    TOK_I8,
    TOK_I16,
    TOK_I32,

    TOK_KW_STRING,
    TOK_KW_BOOLEAN,

    TOK_EOF,
} token_type;

typedef struct {
    token_type type;
    char* span;
    size_t span_size;

    size_t line;
    size_t col;
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

    /**
     * Start of the next token that will be emitted.
     */
    char* start;

    /**
     * Line number at 'start'
     */
    size_t start_line;

    /**
     * Column number at 'start'
     */
    size_t start_col;

    size_t line;
    size_t col;
} lexer_t;

lexer_t make_lexer(char* src, size_t size);
token_result lex_advance();

#endif  // LEX_H
