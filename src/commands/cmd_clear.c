#include <string.h>

#include "cmd_clear.h"
#include "../console_util.h"
#include "../string_util.h"
#include "../core/arith_context.h"

#define CLEAR_CODE      1
#define CLEAR_LAST_CODE 2

int cmd_clear_check(char *input)
{
    if (strcmp("clear", input) == 0) return CLEAR_CODE;
    if (strcmp("clear last", input) == 0) return CLEAR_LAST_CODE;
    return false;
}

/*
Summary: Removes user-defined functions and constants from context
*/
bool cmd_clear_exec(__attribute__((unused)) char *input, int code)
{
    if (code == CLEAR_CODE)
    {
        clear_composite_functions();
        whisper("Functions and constants cleared.\n");
        return true;
    }
    else
    {
        if (get_num_composite_functions() == 0)
        {
            report_error("No functions or constants defined.\n");
            return false;
        }

        pop_composite_function();
        whisper("Removed last function or constant.\n");
        return true;
    }
}
