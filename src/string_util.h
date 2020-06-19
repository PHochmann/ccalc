#pragma once
#include <stdbool.h>
#include <stdarg.h>
#include "parsing/parser.h"

typedef struct
{
    char *buffer;
    size_t buffer_size;
    size_t strlen;
} StringBuilder;

typedef struct
{
    char *string;
    size_t index;
} StringIterator;

bool is_space(char c);
bool is_digit(char c);
bool is_letter(char c);
bool is_opening_parenthesis(char *c);
bool is_closing_parenthesis(char *c);
bool is_delimiter(char *c);
bool begins_with(char *prefix, char *str);
size_t str_split(char *str, char **out_strs, size_t num_delimiters, ...);
char *perr_to_string(ParserError perr);

StringIterator get_iterator(char *string);
char get_next_char(StringIterator *iterator);
size_t console_strlen(char *str);

StringBuilder get_stringbuilder(size_t start_size);
void free_stringbuilder(StringBuilder *builder);
char *append_stringbuilder(StringBuilder *builder, char *fmt, ...);
char *vappend_stringbuilder(StringBuilder *builder, char *fmt, va_list args);
