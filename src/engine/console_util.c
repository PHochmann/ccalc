#include <stdio.h>
#include <string.h>

#include "console_util.h"

char* perr_to_string(ParserError perr)
{
    switch (perr)
    {
        case PERR_SUCCESS:
            return "SUCCESS";
        case PERR_MAX_TOKENS_EXCEEDED:
            return "MAX TOKENS EXCEEDED";
        case PERR_STACK_EXCEEDED:
            return "STACK EXCEEDED";
        case PERR_UNEXPECTED_SUBEXPRESSION:
            return "UNEXPECTED SUBEXPRESSION";
        case PERR_UNEXPECTED_OPENING_PARENTHESIS:
            return "UNEXPECTED OPENING PARENTHESIS";
        case PERR_UNEXPECTED_CLOSING_PARENTHESIS:
            return "UNEXPECTED CLOSING PARENTHESIS";
        case PERR_UNEXPECTED_DELIMITER:
            return "UNEXPECTED DELIMITER";
        case PERR_MISSING_OPERATOR:
            return "UNEXPECTED OPERAND";
        case PERR_MISSING_OPERAND:
            return "MISSING OPERAND";
        case PERR_OUT_OF_MEMORY:
            return "OUT OF MEMORY";
        case PERR_FUNCTION_WRONG_ARITY:
            return "WRONG NUMBER OF OPERANDS FOR FUNCTION";
        case PERR_EMPTY:
            return "EMPTY EXPRESSION";
        default:
            return "UNKNOWN ERROR";
    }
}

/*
Summary: Replaces end of string by three dots if it needed to be shortened because of a limited buffer size
Returns: True if string was changed, false if not
*/
bool indicate_abbreviation(char *string, size_t actual_length)
{
    size_t length = strlen(string);
    if (length < actual_length)
    {
        if (length >= 3)
        {
            string[length - 1] = '.';
            string[length - 2] = '.';
            string[length - 3] = '.';
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool begins_with(char *prefix, char *string)
{
    size_t prefix_length = strlen(prefix);
    size_t string_length = strlen(string);
    if (prefix_length > string_length) return false;
    return strncmp(prefix, string, prefix_length) == 0;
}

void print_padded(char *string, int total_length)
{
    for (int i = strlen(string); i < total_length; i++) printf(" ");
    printf("%s", string);
}

void print_repeated(char* string, int amount)
{
    for (int i = 0; i < amount; i++) printf("%s", string);
}

void print_table(int num_rows, int num_cols, char **cells, bool head_border)
{
    int width[num_cols];
    for (int i = 0; i < num_cols; i++) width[i] = 0;
    
    // Calculate col width
    for (int i = 0; i < num_rows; i++)
    {
        for (int j = 0; j < num_cols; j++)
        {
            int len = strlen(cells[i * num_cols + j]);
            if (width[j] < len) width[j] = len;
        }
    }
    // - - -
    
    // Print top border
    printf("┌");
    for (int i = 0; i < num_cols; i++)
    {
        print_repeated("─", width[i] + 2);
        if (i != num_cols - 1) printf("┬");
    }
    printf("┐\n");
    // - - -
    
    // Print cells
    for (int i = 0; i < num_rows; i++)
    {
        if (i == 1 && head_border) // Print table head border
        {
            printf("├");
            for (int i = 0; i < num_cols; i++)
            {
                print_repeated("─", width[i] + 2);
                if (i != num_cols - 1) printf("┼");
            }
            printf("┤\n");
        }
        
        printf("│");
        for (int j = 0; j < num_cols; j++)
        {
            printf(" ");
            print_padded(cells[i * num_cols + j], width[j]);
            printf(" │");
        }
        printf("\n");
    }
    // - - -
    
    // Print bottom border
    printf("└");
    for (int i = 0; i < num_cols; i++)
    {
        print_repeated("─", width[i] + 2);
        if (i != num_cols - 1) printf("┴");
    }
    printf("┘\n");
    // - - -
}
