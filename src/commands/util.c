#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "util.h"

#define MAX_INPUT_LENGTH 100

void init_util()
{
    rl_bind_key('\t', rl_insert); // Disable tab completion
}

/*
Returns: String representation for user of ParserError
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

/*
Summary: printf-wrapper to filter unimportant prints in non-interactive mode
*/
void whisper(const char *format, ...)
{
    if (g_interactive)
    {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
}

/*
Summary: Used whenever input is requested. Prompt is only printed when interactive.
Params
    file: Used when not interactive - should be stdin when arguments are piped in or file when load command is used
*/
bool ask_input(char *prompt, FILE *file, char **out_input)
{
    if (g_interactive)
    {
        *out_input = readline(prompt);
        if (*out_input == NULL)
        {
            return false;
        }
        add_history(*out_input);
    }
    else
    {
        size_t size = MAX_INPUT_LENGTH;
        *out_input = malloc(MAX_INPUT_LENGTH);
        if (getline(out_input, &size, file) == -1)
        {
            return false;
        }
        (*out_input)[strlen(*out_input) - 1] = '\0'; // Overwrite newline char
    }

    return true;
}

// Debug command:

void debug_init()
{
    g_debug = false;
}

bool debug_check(char *input)
{
    return strcmp(input, "debug") == 0;
}

void debug_exec(__attribute__((unused)) ParsingContext *ctx, __attribute__((unused)) char *input)
{
    g_debug = !g_debug;
    whisper("debug %s\n", g_debug ? "on" : "off");
}

// Quit command:

void quit_init()
{

}

bool quit_check(char *input)
{
    return strcmp(input, "quit") == 0;
}

void quit_exec(__attribute__((unused)) ParsingContext *ctx, __attribute__((unused)) char *input)
{
    exit(0);
}
