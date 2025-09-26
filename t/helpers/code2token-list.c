#include <stdio.h>
#include <string.h>

/* for putting stdout to binary mode on Windows */
#ifdef _WIN32
#include <fcntl.h>
#endif

#include "lex.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <code>\n", argv[0]);
        return 1;
    }

#ifdef _WIN32
    _setmode(_fileno(stdout), _O_BINARY);
#endif

    char* code = argv[1];
    size_t len = strlen(code);

    lexer_t lex = make_lexer(code, len);

    token_result tok;
    while ((tok = lex_advance(&lex)).ok && tok.t.type != TOK_EOF) {
        printf(
            "%ld:%ld %.*s\n",
            tok.t.span.line,
            tok.t.span.col,
            (int)tok.t.span.span_size,
            tok.t.span.span
        );
    }

    return 0;
}
