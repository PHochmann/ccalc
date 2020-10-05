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

bool is_opening_parenthesis(const char *c)
{
    return strcmp(c, "(") == 0 || strcmp(c, "{") == 0;
}

bool is_closing_parenthesis(const char *c)
{
    return strcmp(c, ")") == 0 || strcmp(c, "}") == 0;
}

bool is_delimiter(const char *c)
{
    return strcmp(c, ",") == 0;
}

bool begins_with(const char *prefix, const char *str)
{
    size_t prefix_length = strlen(prefix);
    size_t string_length = strlen(str);
    if (prefix_length > string_length) return false;
    return strncmp(prefix, str, prefix_length) == 0;
}

/*
Summary: Helper function when parsing commands to extract strings between delimiters
    Currently only used in cmd_table_exec
    Will overwrite delimiters in str with \0, does not malloc new out_strs!
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

// Returns: Length of line (excluding \n or \0)
size_t get_line_of_string(const char *string, size_t line_index, char **out_start)
{
    if (string == NULL)
    {
        *out_start = NULL;
        return 0;
    }

    // Search for start of line
    if (line_index > 0)
    {
        while (*string != '\0')
        {
            string++;
            if (*string == '\n')
            {
                line_index--;
                if (line_index == 0)
                {
                    string++;
                    break;
                }
            }
        }
    }

    // String does not have that much lines
    if (line_index != 0)
    {
        return 0;
    }

    *out_start = (char*)string;

    // Count length of line
    size_t count = 0;
    while (string[count] != '\0' && string[count] != '\n')
    {
        count++;
    }
    return count;
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
        case PERR_UNEXPECTED_SUBEXPRESSION:
            return "Unexpected subexpression";
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

char *skip_ansi(const char *str)
{
    if (*str == ESC_START)
    {
        while (*str != ESC_END)
        {
            str++;
        }
        str++;
    }
    return (char*)str;
}
