#pragma once
#include <stdbool.h>
#include <stdarg.h>

#include "vector.h"
#include "../parsing/parser.h"

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

Vector strbuilder_create(size_t start_size);
void strbuilder_reset(Vector *builder);
void strbuilder_append(Vector *builder, char *fmt, ...);
void vstrbuilder_append(Vector *builder, char *fmt, va_list args);
void strbuilder_append_char(Vector *builder, char c);
void strbuilder_reverse(Vector *builder);
