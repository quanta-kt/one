#include "error_printer.h"


/**
 * Default error printer. Formats human readable messages and
 * outputs them to stderr.
 */
static void default_error_printer_fn(
    const char* source, span_info* span, const error_kind kind,
    char const* fmt, va_list args
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

    fprintf(stderr, "Syntax error at %ld:%ld:\n", span->line, span->col);
    vfprintf(stderr, fmt, args);

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

