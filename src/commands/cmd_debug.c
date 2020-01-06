#include <string.h>

#include "cmd_debug.h"
#include "console_util.h"
#include "../tree/parser.h"
#include "../tree/tree_to_string.h"
#include "../arithmetics/arith_context.h"
#include "../string_util.h"

#define COMMAND   "debug "
#define ERROR_FMT "Error: %s.\n"

bool cmd_debug_check(char *input)
{
    return begins_with(COMMAND, input);
}

bool cmd_debug_exec(char *input)
{
    Node *res;
    if (parse_input_from_console(input + strlen(COMMAND), ERROR_FMT, true, &res))
    {
        print_tree_visually(res);
        printf("= ");
        print_tree(res, true);
        printf("\n");
        free_tree(res);
        return true;
    }
    else
    {
        return false;
    }
}
