#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "cmd_evaluation.h"
#include "../util/console_util.h"
#include "../tree/tree_to_string.h"
#include "../tree/tree_util.h"
#include "../core/arith_context.h"
#include "../core/history.h"
#include "../core/evaluation.h"

#define ERROR_FMT        "Error: %s.\n"
#define ASK_VARIABLE_FMT "%s? "

int cmd_evaluation_check(__attribute__((unused)) char *input)
{
    return true;
}

/*
Summary: The evaluation command is executed when input is no other command (hence last in command array at commands.c)
*/
bool cmd_evaluation_exec(char *input, __attribute__((unused)) int code)
{
    Node *res;
    if (arith_parse_input(input, ERROR_FMT, true, &res))
    {
        // Make expression constant by asking for values and binding them to variables
        char *vars[count_variables(res, false)];
        size_t num_vars = list_variables(res, vars);

        /*
        * Ask for variables interactively when we are connected to a terminal
        * When connected to a pipe, it binds variables silently
        * When expression was loaded from a file at a terminal, it asks interactively
        */
        bool temp = set_interactive(isatty(STDIN_FILENO));

        for (size_t i = 0; i < num_vars; i++)
        {
            char *input;
            if (ask_input(stdin, &input, ASK_VARIABLE_FMT, vars[i]))
            {
                Node *res_var;
                if (!arith_parse_input(input, ERROR_FMT, true, &res_var))
                {
                    // Error while parsing - ask again
                    free(input);
                    i--;
                    continue;
                }
                free(input);
                
                if (count_variables(res_var, false) > 0)
                {
                    // Not a constant given - ask again
                    report_error("Not a constant expression.\n");
                    free_tree(res_var);
                    i--;
                    continue;
                }
                
                replace_variable_nodes(&res, res_var, vars[i]);
                free_tree(res_var);
            }
            else
            {
                // EOF when asked for constant
                printf("\n");
                set_interactive(temp);
                free_tree(res);
                return false;
            }
        }
        // Restore previous value of g_interactive
        set_interactive(temp);

        // Print result
        ConstantType result = arith_evaluate(res);
        whisper("= ");
        printf(CONSTANT_TYPE_FMT "\n", result);
        core_update_history(result);
        free_tree(res);
        
        return true;
    }
    else
    {
        return false;
    }
}
