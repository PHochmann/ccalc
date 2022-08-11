#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "../../util/console_util.h"
#include "../../engine/tree/tree_to_string.h"
#include "../../engine/tree/tree_util.h"
#include "../../util/string_util.h"

#include "cmd_evaluation.h"
#include "../core/arith_context.h"
#include "../core/history.h"
#include "../core/arith_evaluation.h"

#define SHOW_CMD "show "

int cmd_evaluation_check(__attribute__((unused)) const char *input)
{
    return true;
}

/*
Summary: The evaluation command is executed when input is no other command (hence last in command array in commands.c)
*/
bool cmd_evaluation_exec(char *input, __attribute__((unused)) int code)
{
    bool show = false;

    if (begins_with(SHOW_CMD, input))
    {
        input += strlen(SHOW_CMD);
        show = true;
    }

    Node *node;
    if (!arith_parse(input, 0, &node)) return false;
    whisper("= ");
    print_tree(node, false);
    printf("\n");
    if (get_type(node) == NTYPE_CONSTANT)
    {
        history_add(get_const_value(node));
    }

    if (show)
    {
        print_tree_visually(node);
    }

    free_tree(node);
    return true;
}
