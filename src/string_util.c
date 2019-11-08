#include <stdarg.h>
#include <string.h>
#include <stdbool.h>

#include "string_util.h"

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
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
}

bool is_opening_parenthesis(char *c)
{
    return strcmp(c, "(") == 0 || strcmp(c, "{") == 0 || strcmp(c, "[") == 0;
}

bool is_closing_parenthesis(char *c)
{
    return strcmp(c, ")") == 0 || strcmp(c, "}") == 0 || strcmp(c, "]") == 0;
}

bool is_delimiter(char *c)
{
    return strcmp(c, ",") == 0;
}

/*
Summary: Calculates length of string displayed in console,
    i.e. reads until \0 or \n and omits ANSI-escaped color sequences
    Todo: Consider \t and other special chars
*/
size_t ansi_strlen(char *str)
{
    size_t res = 0;
    size_t pos = 0;
    while (str[pos] != '\0' && str[pos] != '\n')
    {
        if (str[pos] == 27) // Escape byte (27)
        {
            // Search for terminating byte and abort on end of string
            // (should not happen on well-formed strings but could happen due to truncation)
            while (str[pos] != 'm' && str[pos + 1] != '\0')
            {
                pos++;
            }
        }
        else
        {
            res++;
        }
        pos++;
    }
    return res;
}

bool begins_with(char *prefix, char *str)
{
    size_t prefix_length = strlen(prefix);
    size_t string_length = strlen(str);
    if (prefix_length > string_length) return false;
    return strncmp(prefix, str, prefix_length) == 0;
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

// Currently not in use
/*void trim(char *str)
{
    while (*str == ' ') str++;
    char *end = str + strlen(str) - 1;
    while (end > str && *end == ' ') end--;
    end[1] = '\0';
}*/
