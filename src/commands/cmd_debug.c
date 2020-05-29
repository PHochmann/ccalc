#include <string.h>
#include <stdio.h>

#include "cmd_debug.h"
#include "../string_util.h"
#include "../tree/tree_to_string.h"
#include "../core/arith_context.h"

#define DEBUG "debug "
#define SHOW  "show "
#define DEBUG_CODE 1
#define SHOW_CODE  2
#define ERROR_FMT "Error: %s.\n"

int cmd_debug_check(char *input)
{
    if (begins_with(DEBUG, input)) return DEBUG_CODE;
    if (begins_with(SHOW, input)) return SHOW_CODE;
    return false;
}

bool cmd_debug_exec(char *input, int code)
{
    if (code == DEBUG_CODE)
    {
        input += strlen(DEBUG);
    }
    else
    {
        if (code == SHOW_CODE) input += strlen(SHOW);
    }

    Node *res;
    if (core_parse_input(input, ERROR_FMT, true, &res))
    {
        if (code == DEBUG_CODE)
        {
            print_tree_visually(res);
        }
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
