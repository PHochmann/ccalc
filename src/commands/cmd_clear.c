#include <string.h>

#include "cmd_clear.h"
#include "console_util.h"
#include "../arithmetics/arith_context.h"
#include "../arithmetics/arith_rules.h"

bool cmd_clear_check(char *input)
{
    return strcmp(input, "clear") == 0;
}

/*
Summary: Removes all user-defined functions and constants from context
*/
void cmd_clear_exec(__attribute__((unused)) char *input)
{
    arith_reset_ctx();   // To remove user-defined functions from parsing context
    arith_reset_rules(); // To remove their elimination rules
    whisper("Constants and functions cleared.\n");
}
