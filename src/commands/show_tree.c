#include <string.h>

#include "../engine/parser.h"
#include "../engine/string_util.h"
#include "show_tree.h"
#include "console_util.h"

#define COMMAND "tree "

void show_tree_init() { }

bool show_tree_check(char *input)
{
    return begins_with(COMMAND, input);
}

void show_tree_exec(ParsingContext *ctx, char *input)
{
    input += strlen(COMMAND);
    Node *res;
    if (parse_input_wrapper(ctx, input, &res, false, true, false))
    {
        print_tree_visual(ctx, res);
    }
}
