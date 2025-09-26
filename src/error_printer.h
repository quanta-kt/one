/*
 * The error reporting interface.
 *
 * When parser encouters an error, it calls it's errror printer to report
 * it to user. The default implementation outputs a human readable message
 * to stderr. However, sometimes it might be valuealbe to swap out error
 * printer with an implementaion that outputs in a different format. A binary 
 * used in tests for example might use an implementation that outputs errors
 * in a machine parsable format to make assertions about the errors reported.
 */

#ifndef ERROR_PRINTER_H
#define ERROR_PRINTER_H

#include "lex.h"

#include <stdarg.h>
#include <stdio.h>

typedef enum {
    ERROR_KIND_SYNTAX,
} error_kind;

typedef void (*fn_error)(
    const char* source,
    span_info* span,
    const error_kind kind,
    char const* fmt,
    va_list args
);

typedef struct {
    fn_error error;
} error_printer_t;

extern error_printer_t default_error_printer;

#endif // ERROR_PRINTER_H
