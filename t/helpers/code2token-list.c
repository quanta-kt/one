#include <stdio.h>
#include <string.h>

#include "lex.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <code>\n", argv[0]);
        return 1;
    }

    char* code = argv[1];
    size_t len = strlen(code);

    lexer_t lex = make_lexer(code, len);

    token_result tok;
    while ((tok = lex_advance(&lex)).ok && tok.t.type != TOK_EOF) {
        printf(
            "%ld:%ld %.*s\n",
            tok.t.line,
            tok.t.col,
            (int)tok.t.span_size,
            tok.t.span
        );
    }

    return 0;
}
