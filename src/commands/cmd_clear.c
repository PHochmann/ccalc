#include <stdio.h>
#include <string.h>

#include "cmd_clear.h"
#include "../console_util.h"

#include "../arithmetics/arith_context.h"
#include "../arithmetics/arith_rules.h"
#include "../parsing/operator.h"
#include "../string_util.h"

bool cmd_clear_check(char *input)
{
    return strcmp(input, "clear") == 0;
}

/*
Summary: Prints a string representation for currently defined rules
*/
void cmd_clear_exec(__attribute__((unused)) char *input)
{
    arith_reset_ctx(); // To remove user-defined functions from parsing context
    arith_reset_rules(); // To remove any user-defined rules
    whisper("Constants and functions cleared.\n");
}
