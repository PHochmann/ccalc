#include <stdio.h>
#include <string.h>

#include "show_rules.h"
#include "arith_context.h"
#include "arith_rules.h"
#include "assignments.h"
#include "console_util.h"
#include "../engine/operator.h"
#include "../engine/string_util.h"

void show_rules_init() { }

bool show_rules_check(char *input)
{
    return strcmp(input, "rules") == 0 || strcmp(input, "rules clear") == 0;
}

/*
Summary: Prints a string representation for currently defined rules
*/
void show_rules_exec(ParsingContext *ctx, char *input)
{
    if (strcmp(input, "rules") == 0)
    {
        if (g_num_rules == ARITH_NUM_PREDEFINED_RULES)
        {
            printf("No rules defined.\n");
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
    else // input must be "rules clear"
    {
        arith_reset_ctx(); // To remove user-defined functions from parsing context
        arith_reset_rules(); // To remove any user-defined rules
        printf("Rules and functions cleared.\n");
    }
}
