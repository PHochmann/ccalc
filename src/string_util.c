#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

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
Summary: Helper function to parse commands to extract strings between delimiters
    Currently only used in cmd_table_exec
Example: split("abc x def ghi", out, 2, " x ", " g") = 3
    out = { "abc", "def", "hi" }
*/
size_t split(char *str, char **out_strs, size_t num_delimiters, ...)
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

// Currently not in use
/*void trim(char *str)
{
    while (*str == ' ') str++;
    char *end = str + strlen(str) - 1;
    while (end > str && *end == ' ') end--;
    end[1] = '\0';
}*/
