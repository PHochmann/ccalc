#pragma once
#include <stdbool.h>
#include <stdarg.h>
#include "../parsing/parser.h"

bool is_space(char c);
bool is_digit(char c);
bool is_letter(char c);
bool is_opening_parenthesis(char *c);
bool is_closing_parenthesis(char *c);
bool is_delimiter(char *c);
bool begins_with(char *prefix, char *str);
size_t str_split(char *str, char **out_strs, size_t num_delimiters, ...);
size_t get_line_of_string(char *string, size_t line_index, char **out_start);
char *perr_to_string(ParserError perr);
char *skip_ansi(char *str);
size_t console_strlen(char *str);
