#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "test.h"

char *create_error(char *fmt, ...)
{
    va_list args;
    va_list args_copy;
    va_start(args, fmt);
    va_copy(args_copy, args);
    size_t needed = vsnprintf(NULL, 0, fmt, args) + 1;
    char *res = malloc(needed);
    vsnprintf(res, needed, fmt, args_copy);
    va_end(args);
    va_end(args_copy);
    return res;
}
