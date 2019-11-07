#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "cmd_evaluation.h"

#include "../util/console_util.h"
#include "../util/tree_to_string.h"
#include "../parsing/node.h"
#include "../arithmetics/arith_context.h"
#include "../arithmetics/arith_rules.h"

#define ERROR_FMT        "Error: %s.\n"
#define ASK_VARIABLE_FMT "%s? > "

bool cmd_evaluation_check(__attribute__((unused)) char *input)
{
    return true;
}

/*
Summary: The evaluation command is executed when input is no other command (hence last in command array at core.c)
*/
void cmd_evaluation_exec(char *input)
{
    Node *res;
    if (parse_input_from_console(input, ERROR_FMT, &res))
    {
        // Make expression constant by asking for values and binding them to variables
        char *vars[count_variables(res)];
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
                if (!parse_input_from_console(input, ERROR_FMT, &res_var))
                {
                    // Error while parsing - ask again
                    free(input);
                    i--;
                    continue;
                }
                free(input);
                
                if (count_variables(res_var) > 0)
                {
                    // Not a constant given - ask again
                    printf("Not a constant expression.\n");
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
                return;
            }
        }
        // Restore previous value of g_interactive
        set_interactive(temp);

        ConstantType result = arith_eval(res);
        printf(g_interactive ? "= " CONSTANT_TYPE_FMT "\n" : CONSTANT_TYPE_FMT "\n", result);
        update_ans(result);
        free_tree(res);
    }
}
