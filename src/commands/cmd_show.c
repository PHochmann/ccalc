#include <string.h>
#include <stdio.h>

#include "cmd_show.h"
#include "../util/string_util.h"
#include "../tree/tree_to_string.h"
#include "../tree/tree_util.h"
#include "../core/arith_context.h"
#include "../core/evaluation.h"

#define DEBUG_CMD "debug "
#define SHOW_CMD  "show "
#define DEBUG_CODE 1
#define SHOW_CODE  2
#define ERROR_FMT "Error: %s.\n"

int cmd_show_check(char *input)
{
    if (begins_with(DEBUG_CMD, input)) return DEBUG_CODE;
    if (begins_with(SHOW_CMD, input)) return SHOW_CODE;
    return false;
}

bool cmd_show_exec(char *input, int code)
{
    if (code == DEBUG_CODE)
    {
        input += strlen(DEBUG_CMD);
    }
    else
    {
        if (code == SHOW_CODE) input += strlen(SHOW_CMD);
    }

    Node *res;
    if (core_parse_input(input, ERROR_FMT, true, &res, code == DEBUG_CODE))
    {
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
