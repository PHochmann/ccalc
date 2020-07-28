#include <string.h>

#include "cmd_clear.h"
#include "../util/console_util.h"
#include "../util/string_util.h"
#include "../core/arith_context.h"

#define CLEAR_CODE 1
#define UNDEF_CODE 2

#define CLEAR_COMMAND "clear"
#define UNDEF_COMMAND "undef "

int cmd_clear_check(char *input)
{
    if (strcmp(CLEAR_COMMAND, input) == 0) return CLEAR_CODE;
    if (begins_with(UNDEF_COMMAND, input)) return UNDEF_CODE;
    return false;
}

/*
Summary: Removes user-defined functions and constants from context
*/
bool cmd_clear_exec(char *input, int code)
{
    if (code == CLEAR_CODE)
    {
        clear_composite_functions();
        whisper("Functions and constants cleared.\n");
        return true;
    }
    else
    {
        input += strlen(UNDEF_COMMAND);
        
        Operator *function = ctx_lookup_op(g_ctx, input, OP_PLACE_FUNCTION);
        if (function != NULL)
        {
            if (remove_composite_function(function))
            {
                whisper("Function or constant removed.\n");
                return true;
            }
            else
            {
                report_error("Predefined operators can not be removed.\n");
                return false;
            }
        }
        else
        {
            report_error("Unknown function.\n");
            return false;
        }
    }
}
