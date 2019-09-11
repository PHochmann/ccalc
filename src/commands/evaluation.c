#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "arith_context.h"
#include "arith_rules.h"
#include "evaluation.h"
#include "core.h"
#include "util.h"
#include "assignments.h"

#include "../engine/string_util.h"

#define ANS_VAR                 "ans"
#define TTY_ASK_VARIABLE_PROMPT "? > "

static const size_t MAX_DEBUG_LENGTH = 500;
static const size_t MAX_ITERATIONS   = 50;

void evaluation_init()
{
    g_ans = NULL;
}

bool evaluation_check(__attribute__((unused)) char *input)
{
    return true;
}

bool parse_input_wrapper(ParsingContext *ctx, char *input, Node **out_res, bool apply_rules, bool apply_ans, bool constant)
{
    ParserError perr = parse_input(ctx, input, out_res);
    if (perr != PERR_SUCCESS)
    {
        if (perr != PERR_EMPTY)
        {
            printf("Error: %s\n", perr_to_string(perr));
        }
        
        return false;
    }
    
    if (apply_ans && g_ans != NULL) tree_substitute_var(ctx, *out_res, g_ans, ANS_VAR);
    if (apply_rules)
    {
        size_t appliances = apply_ruleset(*out_res, g_num_rules, g_rules, MAX_ITERATIONS);
        if (appliances == MAX_ITERATIONS) whisper("Warning: Possibly non-terminating ruleset\n");
    }

    // Make expression constant by asking for values and binding them to variables
    if (constant)
    {
        char *vars[MAX_VAR_COUNT];
        size_t num_variables = tree_list_vars(*out_res, vars);

        for (size_t i = 0; i < num_variables; i++)
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
                if (!parse_input_wrapper(ctx, input, &res_var, apply_rules, apply_ans, false))
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
                // EOL when asked for constant
                printf("\n");
                set_interactive(temp);
                return false;
            }

            set_interactive(temp);
        }
    }
    
    return true;
}

/*
Summary: The evaluation command is executed when input is no other command (hence last in command array at core.c)
*/
void evaluation_exec(ParsingContext *ctx, char *input)
{
    Node *res;
    
    if (parse_input_wrapper(ctx, input, &res, true, true, true))
    {
        // Show AST and string when debug=true
        if (g_debug)
        {
            print_tree_visual(ctx, res);
            char inlined_tree[MAX_DEBUG_LENGTH];
            size_t size = tree_inline(ctx, res, inlined_tree, MAX_DEBUG_LENGTH, true);
            indicate_abbreviation(inlined_tree, size);
            printf("= %s\n", inlined_tree);
        }
        
        double eval = arith_eval(res);
        char result_str[ctx->min_str_len];
        ctx->to_string((void*)(&eval), result_str);

        if (g_interactive)
        {
            printf("= %s\n", result_str);
        }
        else
        {
            printf("%s\n", result_str);
        }
        
        if (g_ans != NULL) free_tree(g_ans);
        g_ans = res;
    }
}
