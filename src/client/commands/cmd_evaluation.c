#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "../../engine/util/console_util.h"
#include "../../engine/tree/tree_to_string.h"
#include "../../engine/tree/tree_util.h"

#include "cmd_evaluation.h"
#include "../core/arith_context.h"
#include "../core/history.h"
#include "../core/arith_evaluation.h"

#define MAX_VARIABLES_DISTINCT 10

#define ERROR_FMT        "Error: %s\n"
#define ASK_VARIABLE_FMT "%s? "

int cmd_evaluation_check(__attribute__((unused)) const char *input)
{
    return true;
}

/*
Summary: The evaluation command is executed when input is no other command (hence last in command array in commands.c)
*/
bool cmd_evaluation_exec(char *input, __attribute__((unused)) int code)
{
    Node *tree;
    if (arith_parse_and_postprocess(input, ERROR_FMT, &tree))
    {
        // Make expression constant by asking for values and binding them to variables
        const char *vars[MAX_VARIABLES_DISTINCT];
        bool sufficient_buff = false;
        ssize_t num_vars = list_variables(tree, MAX_VARIABLES_DISTINCT, vars, &sufficient_buff);
        if (!sufficient_buff)
        {
            report_error("Too many variables.\n");
            free_tree(tree);
            return false;
        }

        /*
        * Ask for variables interactively when we are connected to a terminal
        * When connected to a pipe, it binds variables silently
        * When expression was loaded from a file at a terminal, it asks interactively
        */
        bool temp = set_interactive(isatty(STDIN_FILENO));

        for (size_t i = 0; i < (size_t)num_vars; i++)
        {
            char *input;
            if (ask_input(stdin, &input, ASK_VARIABLE_FMT, vars[i]))
            {
                Node *tree_var;
                if (!arith_parse_and_postprocess(input, ERROR_FMT, &tree_var))
                {
                    // Error while parsing - ask again
                    free(input);
                    i--;
                    continue;
                }
                free(input);
                
                if (count_all_variable_nodes(tree_var) > 0)
                {
                    // Not a constant given
                    report_error("Error: Not a constant expression\n");
                    free_tree(tree_var);
                    free_tree(tree);
                    set_interactive(temp);
                    return false;
                }
                
                replace_variable_nodes(&tree, tree_var, vars[i]);
                free_tree(tree_var);
            }
            else
            {
                // EOF when asked for constant
                printf("\n");
                set_interactive(temp);
                free_tree(tree);
                return false;
            }
        }
        // Restore previous value of g_interactive
        set_interactive(temp);

        // Print result
        double result = 0;
        ListenerError l_err = tree_reduce(tree, arith_op_evaluate, &result);
        if (l_err == LISTENERERR_SUCCESS)
        {
            whisper("= ");
            printf(CONSTANT_TYPE_FMT "\n", result);
            history_add(result);
            free_tree(tree);
            return true;
        }
        else
        {
            report_error("Error: %s\n", listenererr_to_str(l_err));
            free_tree(tree);
            return false;
        }
    }
    else
    {
        return false;
    }
}
