#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "util.h"
#include "evaluation.h"
#include "assignments.h"
#include "../engine/context.h"

#define MAX_ITERATIONS 50
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
            return "SUCCESS";
        case PERR_MAX_TOKENS_EXCEEDED:
            return "MAX TOKENS EXCEEDED";
        case PERR_STACK_EXCEEDED:
            return "STACK EXCEEDED";
        case PERR_UNEXPECTED_SUBEXPRESSION:
            return "UNEXPECTED SUBEXPRESSION";
        case PERR_EXCESS_OPENING_PARENTHESIS:
            return "MISSING CLOSING PARENTHESIS";
        case PERR_EXCESS_CLOSING_PARENTHESIS:
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
        case PERR_CHILDREN_EXCEEDED:
            return "EXCEEDED MAXIMUM NUMBER OF OPERANDS FOR FUNCTION"; 
        case PERR_EMPTY:
            return "EMPTY EXPRESSION";
        default:
            return "UNKNOWN ERROR";
    }
}

/*
Summary: printf-wrapper to filter unimportant prints in silent mode
*/
void whisper(const char *format, ...)
{
    if (!g_silent)
    {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
}

/*
Summary: Used whenever user input is requested. Prompt is only printed when not silent.
*/
bool ask_input(char *prompt, char **out_input)
{
    if (g_silent) prompt = "";
    *out_input = readline(prompt);
    if (!(*out_input)) return false;
    add_history(*out_input);
    return true;
}

bool parse_input_wrapper(ParsingContext *ctx, char *input, bool pad_parentheses, Node **out_res, bool apply_rules, bool apply_ans, bool constant)
{
    ParserError perr = parse_input(ctx, input, pad_parentheses, out_res);
    if (perr != PERR_SUCCESS)
    {
        printf("Error: %s\n", perr_to_string(perr));
        return false;
    }
    
    if (apply_ans && g_ans != NULL) tree_substitute_var(ctx, *out_res, g_ans, "ans");
    if (apply_rules)
    {
        int appliances = apply_ruleset(*out_res, g_num_rules, g_rules, MAX_ITERATIONS);
        if (appliances == MAX_ITERATIONS) whisper("Warning: Possibly non-terminating ruleset\n");
    }

    // Make expression constant by asking for values and binding them to variables
    if (constant)
    {
        char *vars[MAX_VAR_COUNT];
        int num_variables = tree_list_vars(*out_res, vars);
        for (int i = 0; i < num_variables; i++)
        {
            printf(" %s? ", vars[i]);
            char *input;
            if (ask_input("> ", &input))
            {
                Node *res_var;
                if (!parse_input_wrapper(ctx, input, pad_parentheses, &res_var, apply_rules, apply_ans, false))
                {
                    // Error while parsing - ask again
                    free(input);
                    i--;
                    continue;
                }
                free(input);
                
                if (tree_contains_vars(res_var))
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
            else return false;
        }
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
