#pragma once
#include <stdlib.h>
#include <stdarg.h>
#include "vector.h"

// A StringBuilder is just a Vector of chars
typedef Vector StringBuilder;

StringBuilder strbuilder_create(size_t start_size);
void strbuilder_clear(StringBuilder *builder);
void strbuilder_append(StringBuilder *builder, const char *fmt, ...);
void vstrbuilder_append(StringBuilder *builder, const char *fmt, va_list args);
void strbuilder_append_char(StringBuilder *builder, char c);
char *strbuilder_to_str(const StringBuilder *builder);
