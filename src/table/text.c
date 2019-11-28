#include <stdio.h>
#include "text.h"
#include "table.h"

size_t get_text_width(char *str)
{
    size_t res = 0;
    for (size_t i = 0; i < get_num_lines(str); i++)
    {
        char *line;
        get_line_of_string(str, i, &line);
        size_t line_width = console_strlen(line);
        if (line_width > res) res = line_width;
    }
    return res;
}

/*
Summary: Calculates length of string displayed in console,
    i.e. reads until \0 or \n and omits ANSI-escaped color sequences
    Todo: Consider \t and other special chars
*/
size_t console_strlen(char *str)
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

void print_repeated(char *string, size_t times)
{
    for (size_t i = 0; i < times; i++) printf("%s", string);
}

void print_padded(char *string, int bytes, int total_length, int dot_padding, TextAlignment textpos)
{
    if (string == NULL)
    {
        printf("%*s", total_length, "");
        return;
    }

    // Lengths passed to printf are including color codes
    // We need to adjust the total length to include them
    int string_length = console_strlen(string);
    int adjusted_total_len = total_length + bytes - string_length;

    switch (textpos)
    {
        case ALIGN_LEFT:
        {
            printf("%-*.*s", adjusted_total_len, bytes, string);
            break;
        }
        case ALIGN_RIGHT:
        {
            printf("%*.*s", adjusted_total_len, bytes, string);
            break;
        }
        case ALIGN_CENTER:
        {
            int padding = (total_length - string_length) / 2;
            printf("%*s%.*s%*s", padding, "", bytes, string,
                (total_length - string_length) % 2 == 0 ? padding : padding + 1, "");
            break;
        }
        case ALIGN_NUMBERS:
        {
            printf("%*s%.*s%*s", dot_padding, "", bytes, string,
                total_length - string_length - dot_padding, "");
            break;
        }
    }
}

size_t get_num_lines(char *string)
{
    size_t res = 1;
    size_t pos = 0;
    while (string[pos] != '\0')
    {
        if (string[pos] == '\n')
        {
            res++;
        }
        pos++;
    }
    return res;
}

// Returns: Length of line (excluding \n or \0)
size_t get_line_of_string(char *string, size_t line_index, char **out_start)
{
    if (string == NULL)
    {
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

    *out_start = string;

    // Count length of line
    size_t count = 0;
    while (string[count] != '\0' && string[count] != '\n')
    {
        count++;
    }
    return count;
}
