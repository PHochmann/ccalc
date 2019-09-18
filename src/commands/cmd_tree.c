#include <string.h>

#include "../parsing/parser.h"
#include "../string_util.h"
#include "cmd_tree.h"
#include "console_util.h"

#define COMMAND "tree "

void cmd_tree_init() { }

bool cmd_tree_check(char *input)
{
    return begins_with(COMMAND, input);
}

void cmd_tree_exec(ParsingContext *ctx, char *input)
{
    input += strlen(COMMAND);
    Node *res;
    if (parse_input_wrapper(ctx, input, &res, false, true, false))
    {
        print_tree_visual(ctx, res);
    }
}
