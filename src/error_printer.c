#include "error_printer.h"

const error_desc_t E0001 = { .code = 001, .message_fmt = "Unbalanced braces" };
const error_desc_t E0002 = { .code = 002, .message_fmt = "Unexpected character: '%c'" };
const error_desc_t E0003 = { .code = 003, .message_fmt = "Unterminated string: %.*s", };

const error_desc_t E0501 = { .code = 501, .message_fmt = "Syntax error" };
const error_desc_t E0502 = { .code = 502, .message_fmt = "Expected a type" };
const error_desc_t E0503 = { .code = 503, .message_fmt = "EOF while parsing function type params" };
const error_desc_t E0504 = { .code = 504, .message_fmt = "EOF while parsing tuple items" };
const error_desc_t E0505 = { .code = 505, .message_fmt = "expected '(' after 'fn'" };
const error_desc_t E0506 = { .code = 506, .message_fmt = "expected function declaration" };
const error_desc_t E0507 = { .code = 507, .message_fmt = "expected a parameter name" };
const error_desc_t E0508 = { .code = 508, .message_fmt = "expected ':' after parameter name" };
const error_desc_t E0509 = { .code = 509, .message_fmt = "expected an identifier after 'fn'" };
const error_desc_t E0510 = { .code = 510, .message_fmt = "expected a '(' after function name" };
const error_desc_t E0511 = { .code = 511, .message_fmt = "expected a ')' after function params" };
const error_desc_t E0512 = { .code = 512, .message_fmt = "expected function body" };
const error_desc_t E0513 = { .code = 513, .message_fmt = "expected identifier after 'let mut'" };
const error_desc_t E0514 = { .code = 514, .message_fmt = "expected identifier after 'let'" };
const error_desc_t E0515 = { .code = 515, .message_fmt = "expected ';' after variable declaration" };
const error_desc_t E0516 = { .code = 516, .message_fmt = "unclosed block" };
const error_desc_t E0517 = { .code = 517, .message_fmt = "expected '{' after if" };
const error_desc_t E0518 = { .code = 518, .message_fmt = "expected 'if' or '{' after else" };
const error_desc_t E0519 = { .code = 519, .message_fmt = "expected '{' after while" };
const error_desc_t E0520 = { .code = 520, .message_fmt = "expected ';' after statement" };
const error_desc_t E0521 = { .code = 521, .message_fmt = "can only assign to identifiers" };
const error_desc_t E0522 = { .code = 522, .message_fmt = "expected a ')' after function arguments" };
const error_desc_t E0523 = { .code = 523, .message_fmt = "expected primary expression" };
const error_desc_t E0524 = { .code = 524, .message_fmt = "expected ')'" };
const error_desc_t E0525 = { .code = 525, .message_fmt = "invalid escape sequence" };
const error_desc_t E0526 = { .code = 526, .message_fmt = "unexpected closing delimiter '}'" };
const error_desc_t E0527 = { .code = 527, .message_fmt = "unclosed delimiter '{'" };


/**
 * Default error printer. Formats human readable messages and
 * outputs them to stderr.
 */
static void default_error_printer_fn(
    const char* source, span_info* span, const error_desc_t error,
    va_list args
) {
    char* line_start = span->span;
    char* line_end = span->span;
    int line_len;
    size_t tabs_count = 0;

    /**
     * Determine the start and the end of the line where token is present.
     * We rely on the fact that token's span is a slice of the entire source
     * program, i.e. the 'src' paramater.
     */
    while (line_start != source && *(line_start - 1) != '\n') {
        if (*line_start == '\t') {
            tabs_count++;
        }

        line_start--;
    }

    while (*line_end != '\0' && *line_end != '\n') {
        line_end++;
    }

    line_len = (int)(line_end - line_start);

    fprintf(stderr, "%ld:%ld error (E%04d):\n", span->line, span->col, error.code);
    vfprintf(stderr, error.message_fmt, args);

    fprintf(
        stderr,
        "\n%5ld | %.*s%s\n",
        span->line,
        line_len,
        line_start,
        *line_end == '\0' ? "(end of file)" : ""
    );

    /*
     * When dealing with tabs, we must pad with tabs instead of spaces
     * to ensure correct alignment of '^' with the token it is pointing at.
     */
    for (size_t i = 0; i < tabs_count; i++) {
        fputc('\t', stderr);
    }

    for (size_t i = tabs_count; i < span->col + 7; i++) {
        fputc(' ', stderr);
    }

    for (size_t i = 0; i < span->span_size; i++) {
        fputc('^', stderr);
    }

    fputc('\n', stderr);
}

error_printer_t default_error_printer = {
    .error = default_error_printer_fn,
};

