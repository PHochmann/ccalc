#include <string.h>

#include "cmd_clear.h"
#include "console_util.h"
#include "../string_util.h"
#include "../arithmetics/arith_context.h"
#include "../arithmetics/arith_rules.h"

bool cmd_clear_check(char *input)
{
    return begins_with("clear", input);
}

/*
Summary: Removes all user-defined functions and constants from context
*/
void cmd_clear_exec(__attribute__((unused)) char *input)
{
    if (g_num_rules == ARITH_NUM_RULES)
    {
        printf("No functions or constants defined.\n");
        return;
    }

    if (strcmp("clear", input) == 0)
    {
        arith_reset_ctx();   // To remove user-defined functions from parsing context
        arith_reset_rules(); // To remove their elimination rules
        whisper("Functions and constants cleared.\n");
    }
    else
    {
        if (strcmp("clear last", input) == 0)
        {
            g_num_rules--;
            g_ctx->num_ops--;
            free(g_ctx->operators[g_ctx->num_ops].name);
            free_rule(g_rules[g_num_rules]);
            whisper("Last function or constant removed.\n");
        }
        else
        {
            printf("Syntax error.\n");
        }
    }
}
