#include <string.h>

#include "cmd_clear.h"
#include "console_util.h"
#include "../string_util.h"
#include "../arithmetics/arith_context.h"
#include "../arithmetics/arith_transformation.h"

bool cmd_clear_check(char *input)
{
    return strcmp("clear", input) == 0 || strcmp("clear last", input) == 0;
}

void pop_rule_and_op()
{
    arith_pop_rule();
    g_ctx->num_ops--;
    free(g_ctx->operators[g_ctx->num_ops].name);
}

/*
Summary: Removes user-defined functions and constants from context
*/
bool cmd_clear_exec(char *input)
{
    if (strcmp("clear", input) == 0)
    {
        while (arith_get_num_userdefined() != 0)
        {
            pop_rule_and_op();
        }
        
        whisper("Functions and constants cleared.\n");
        return true;
    }
    else
    {
        if (arith_get_num_userdefined() == 0)
        {
            printf("No functions or constants defined.\n");
            return false;
        }

        pop_rule_and_op();

        whisper("Removed last function or constant.\n");
        return true;
    }
}
