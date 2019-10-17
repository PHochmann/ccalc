#include <string.h>

#include "../parsing/parser.h"
#include "../arithmetics/arith_context.h"
#include "../util/string_util.h"
#include "../util/console_util.h"
#include "cmd_debug.h"

#define COMMAND "debug "

bool cmd_debug_check(char *input)
{
    return begins_with(COMMAND, input);
}

void cmd_debug_exec(char *input)
{
    Node *res;
    if (parse_input_from_console(input + strlen(COMMAND), "Error: %s\n", &res))
    {
        print_tree_visual(res);
        printf("= ");
        print_tree_inlined(res, true);
        printf("\n");
        free_tree(res);
    }
}
