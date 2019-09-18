#include <stdio.h>
#include <string.h>

#include "cmd_functions.h"
#include "console_util.h"

#include "../arith_context.h"
#include "../arith_rules.h"
#include "../parsing/operator.h"
#include "../string_util.h"

void cmd_functions_init() { }

bool cmd_functions_check(char *input)
{
    return strcmp(input, "functions") == 0 || strcmp(input, "functions clear") == 0;
}

/*
Summary: Prints a string representation for currently defined rules
*/
void cmd_functions_exec(ParsingContext *ctx, char *input)
{
    if (strcmp(input, "functions") == 0)
    {
        if (g_num_rules == ARITH_NUM_PREDEFINED_RULES)
        {
            printf("No functions defined.\n");
            return;
        }

        for (size_t i = ARITH_NUM_PREDEFINED_RULES; i < g_num_rules; i++)
        {
            print_tree_inlined(ctx, g_rules[i].before, true);
            printf(" -> ");
            print_tree_inlined(ctx, g_rules[i].after, true);
            printf("\n");
        }
    }
    else // input must be "functions clear"
    {
        arith_reset_ctx(); // To remove user-defined functions from parsing context
        arith_reset_rules(); // To remove any user-defined rules
        printf("Functions cleared.\n");
    }
}
