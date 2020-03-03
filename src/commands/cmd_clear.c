#include <string.h>

#include "cmd_clear.h"
#include "../console_util.h"
#include "../string_util.h"
#include "../core/arith_context.h"

bool cmd_clear_check(char *input)
{
    return strcmp("clear", input) == 0 || strcmp("clear last", input) == 0;
}

/*
Summary: Removes user-defined functions and constants from context
*/
bool cmd_clear_exec(char *input)
{
    if (strcmp("clear", input) == 0)
    {
        clear_composite_functions();
        whisper("Functions and constants cleared.\n");
        return true;
    }
    else
    {
        if (get_num_composite_functions() == 0)
        {
            printf("No functions or constants defined.\n");
            return false;
        }

        pop_composite_function();
        whisper("Removed last function or constant.\n");
        return true;
    }
}
