#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>

#include "string_util.h"

#define ESC_START  27
#define ESC_END   109

bool is_space(char c)
{
    return c == ' ';
}

bool is_digit(char c)
{
    return (c >= '0' && c <= '9') || c == '.';
}

bool is_letter(char c)
{
    return (c >= 'A' && c <= 'Z')
        || (c >= 'a' && c <= 'z')
        || c == '_'
        || c == '['
        || c == ']';
}

bool is_opening_parenthesis(char *c)
{
    return strcmp(c, "(") == 0 || strcmp(c, "{") == 0;
}

bool is_closing_parenthesis(char *c)
{
    return strcmp(c, ")") == 0 || strcmp(c, "}") == 0;
}

bool is_delimiter(char *c)
{
    return strcmp(c, ",") == 0;
}

bool begins_with(char *prefix, char *str)
{
    size_t prefix_length = strlen(prefix);
    size_t string_length = strlen(str);
    if (prefix_length > string_length) return false;
    return strncmp(prefix, str, prefix_length) == 0;
}

/*
Summary: Helper function when parsing commands to extract strings between delimiters
    Currently only used in cmd_table_exec
Example: split("abc x def ghi", out, 2, " x ", " g") = 3
    out = { "abc", "def", "hi" }
*/
size_t str_split(char *str, char **out_strs, size_t num_delimiters, ...)
{
    va_list args;
    va_start(args, num_delimiters);
    size_t res = 1;
    for (size_t i = 0; i < num_delimiters; i++)
    {
        char *delimiter = va_arg(args, char*);
        char *end_pos = strstr(str, delimiter);

        if (end_pos != NULL)
        {
            *end_pos = '\0';
            out_strs[res - 1] = str;
            res++;
            str = end_pos + strlen(delimiter);
        }
        else
        {
            break;
        }
    }

    out_strs[res - 1] = str;
    return res;
}

/*
Returns: String representation of ParserError
*/
char *perr_to_string(ParserError perr)
{
    switch (perr)
    {
        case PERR_SUCCESS:
            return "Success";
        case PERR_MAX_TOKENS_EXCEEDED:
            return "Max. Tokens exceeded";
        case PERR_STACK_EXCEEDED:
            return "Stack exceeded";
        case PERR_UNEXPECTED_SUBEXPRESSION:
            return "Unexpected Subexpression";
        case PERR_EXCESS_OPENING_PARENTHESIS:
            return "Missing closing parenthesis";
        case PERR_EXCESS_CLOSING_PARENTHESIS:
            return "Unexpected closing parenthesis";
        case PERR_UNEXPECTED_DELIMITER:
            return "Unexpected delimiter";
        case PERR_MISSING_OPERATOR:
            return "Unexpected operand";
        case PERR_MISSING_OPERAND:
            return "Missing operand";
        case PERR_OUT_OF_MEMORY:
            return "Out of memory";
        case PERR_FUNCTION_WRONG_ARITY:
            return "Wrong number of operands of function";
        case PERR_CHILDREN_EXCEEDED:
            return "Exceeded maximum number of operands of function";
        case PERR_EMPTY:
            return "Empty Expression";
        default:
            return "Unknown Error";
    }
}

StringIterator get_iterator(char *string)
{
    return (StringIterator){ .string = string, .index = 0 };
}

char get_next_char(StringIterator *iterator)
{
    if (iterator->string[iterator->index] == ESC_START)
    {
        while (iterator->string[iterator->index] != ESC_END)
        {
            iterator->index++;
        }
        iterator->index++;
    }
    return iterator->string[iterator->index++];
}

/*
Summary: Calculates length of string displayed in console,
    i.e. reads until \0 or \n and omits ANSI-escaped color sequences
    Todo: Consider \t and other special chars
*/
size_t console_strlen(char *str)
{
    if (str == NULL) return 0;
    StringIterator iterator = get_iterator(str);
    size_t res = 0;
    while (true)
    {
        char curr = get_next_char(&iterator);
        if (curr == '\0' || curr == '\n') break;
        res++;
    }
    return res;
}

StringBuilder get_stringbuilder(size_t start_size)
{
    if (start_size == 0) start_size = 10;
    StringBuilder builder = {
        .buffer = malloc(start_size * sizeof(char)),
        .buffer_size = start_size,
        .strlen = 0
    };
    builder.buffer[0] = '\0';
    return builder;
}

void reset_stringbuilder(StringBuilder *builder)
{
    builder->buffer[0] = '\0';
    builder->strlen = 0;
}

void free_stringbuilder(StringBuilder *builder)
{
    free(builder->buffer);
}

char *append_stringbuilder(StringBuilder *builder, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char *res = vappend_stringbuilder(builder, fmt, args);
    va_end(args);
    return res;
}

char *vappend_stringbuilder(StringBuilder *builder, char *fmt, va_list args)
{
    va_list args_copy;
    va_copy(args_copy, args);

    size_t appended_length = vsnprintf(
        builder->buffer + builder->strlen,
        builder->buffer_size - builder->strlen,
        fmt, args);

    bool try_again = false;
    while (builder->buffer_size < builder->strlen + appended_length + 1)
    {
        // Double buffer size as long as size is not sufficient
        builder->buffer_size <<= 1;
        try_again = true;
    }

    if (try_again)
    {
        builder->buffer = realloc(builder->buffer, builder->buffer_size);
        vsnprintf(builder->buffer + builder->strlen, appended_length + 1, fmt, args_copy);
    }
    
    builder->strlen += appended_length;
    va_end(args_copy);
    return builder->buffer;
}
