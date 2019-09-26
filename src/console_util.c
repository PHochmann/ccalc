#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "string_util.h"
#include "arithmetics/arith_rules.h"
#include "console_util.h"

#define COL_RESET   "\033[0m"
#define TTY_ASK_VARIABLE_PROMPT "? > "

static const size_t MAX_INPUT_LENGTH    = 100;
static const size_t MAX_INLINED_LENGTH  = 200;

void init_console_util()
{
    g_interactive = false;
    // Disable tab completion
    rl_bind_key('\t', rl_insert);
}

/*
Returns: String representation of ParserError to print
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
Summary: Sets interactive mode
    When true, whispered messages are displayed and readline instead of getline is used to read input
*/
bool set_interactive(bool value)
{
    bool res = g_interactive;
    g_interactive = value;
    return res;
}

// Summary: Prints result of tree inline, including correct color even after truncation, and indicated abbreviation
void print_tree_inlined(ParsingContext *ctx, Node *node, bool color)
{
    char buffer[MAX_INLINED_LENGTH + 1];
    size_t result = tree_inline(ctx, node, buffer, MAX_INLINED_LENGTH + 1, color);
    printf("%s", buffer);
    if (color) printf(COL_RESET);
    if (result > MAX_INLINED_LENGTH) printf("...");
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
    prompt: Prompt to display when interactive
    file: Used when not interactive - should be stdin when arguments are piped in or file when load command is used
    out_input: Pointer to string that will be read. String must be free'd after use.
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

/*
Summary:
    Parses input, does post-processing of input, gives feedback on command line
Returns:
    True when input was successfully parsed, false when syntax error in input or aborted when asked for constant
*/
bool parse_input_console(ParsingContext *ctx, char *input, char *error_fmt, Node **out_res, bool constant, bool update_ans)
{
    ParserError perr = parse_input(ctx, input, out_res);
    if (perr != PERR_SUCCESS)
    {
        printf(error_fmt, perr_to_string(perr));
        return false;
    };

    char *error_msg = transform_input(ctx, *out_res, update_ans);
    if (error_msg != NULL)
    {
        printf(error_fmt, error_msg);
        free_tree(*out_res);
        return false;
    }

    // Make expression constant by asking for values and binding them to variables
    if (constant)
    {
        char *vars[MAX_VAR_COUNT];
        size_t num_vars;
        
        if (!tree_list_vars(*out_res, &num_vars, vars))
        {
            printf("Error: %s\n", perr_to_string(PERR_STACK_EXCEEDED));
            return false;
        }

        for (size_t i = 0; i < num_vars; i++)
        {
            // Make interactive even when loading a file
            bool temp = set_interactive(isatty(STDIN_FILENO));
            
            // printf'ing prompt beforehand causes overwrite when using arrow keys
            char prompt[strlen(vars[i]) + strlen(TTY_ASK_VARIABLE_PROMPT) + 1];
            sprintf(prompt, "%s" TTY_ASK_VARIABLE_PROMPT, vars[i]);

            char *input;
            if (ask_input(prompt, stdin, &input))
            {
                Node *res_var;
                if (!parse_input_console(ctx, input, error_fmt, &res_var, false, false))
                {
                    // Error while parsing - ask again
                    free(input);
                    i--;
                    continue;
                }
                free(input);
                
                if (tree_count_vars(res_var) > 0)
                {
                    // Not a constant given - ask again
                    printf("Not a constant expression\n");
                    free_tree(res_var);
                    i--;
                    continue;
                }
                
                tree_substitute_var(ctx, *out_res, res_var, vars[i]);
                free(vars[i]);
                free_tree(res_var);
            }
            else
            {
                // EOF when asked for constant
                printf("\n");
                set_interactive(temp);
                return false;
            }

            set_interactive(temp);
        }
    }
    
    return true;
}
