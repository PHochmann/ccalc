#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "util.h"
#include "evaluation.h"
#include "assignments.h"
#include "../engine/context.h"
#include "../engine/parser.h"
#include "../engine/console_util.h"

void init_util()
{
    rl_bind_key('\t', rl_insert); // Disable tab completion
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
Summary: Used whenever user input is requested
*/
bool ask_input(char *prompt, char **out_input)
{
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
        apply_ruleset(*out_res, g_num_rules, g_rules, 50);
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