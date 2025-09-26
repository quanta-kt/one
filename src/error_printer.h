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

typedef struct {
    unsigned int code;
    char* message_fmt;
} error_desc_t;

// Lex errors
extern const error_desc_t E0001;
extern const error_desc_t E0002;
extern const error_desc_t E0003;

// Parse errors
extern const error_desc_t E0501;
extern const error_desc_t E0502;
extern const error_desc_t E0503;
extern const error_desc_t E0504;
extern const error_desc_t E0505;
extern const error_desc_t E0506;
extern const error_desc_t E0507;
extern const error_desc_t E0508;
extern const error_desc_t E0509;
extern const error_desc_t E0510;
extern const error_desc_t E0511;
extern const error_desc_t E0512;
extern const error_desc_t E0513;
extern const error_desc_t E0514;
extern const error_desc_t E0515;
extern const error_desc_t E0516;
extern const error_desc_t E0517;
extern const error_desc_t E0518;
extern const error_desc_t E0519;
extern const error_desc_t E0520;
extern const error_desc_t E0521;
extern const error_desc_t E0522;
extern const error_desc_t E0523;
extern const error_desc_t E0524;
extern const error_desc_t E0525;
extern const error_desc_t E0526;
extern const error_desc_t E0527;


typedef void (*fn_error)(
    const char* source,
    span_info* span,
    const error_desc_t error,
    va_list args
);

typedef struct {
    fn_error error;
} error_printer_t;

extern error_printer_t default_error_printer;

#endif // ERROR_PRINTER_H
