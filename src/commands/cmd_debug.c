#include <string.h>

#include "../parsing/parser.h"
#include "../arithmetics/arith_context.h"
#include "../string_util.h"
#include "../console_util.h"
#include "cmd_debug.h"

#define COMMAND "debug "

bool cmd_debug_check(char *input)
{
    return begins_with(COMMAND, input);
}

void cmd_debug_exec(char *input)
{
    Node res;
    if (parse_input_from_console(g_ctx, input + strlen(COMMAND), "Error: %s\n", &res, false, true))
    {
        print_tree_visual(res);
        printf("= ");
        print_tree_inlined(res, true);
        printf("\n");
        free_tree(res);
    }
}
