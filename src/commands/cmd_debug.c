#include <string.h>

#include "../parsing/parser.h"
#include "../string_util.h"
#include "cmd_debug.h"
#include "console_util.h"

#define COMMAND "debug "

void cmd_debug_init() { }

bool cmd_debug_check(char *input)
{
    return begins_with(COMMAND, input);
}

void cmd_debug_exec(ParsingContext *ctx, char *input)
{
    input += strlen(COMMAND);
    Node *res;
    if (parse_input_console(ctx, input, "Error: %s", &res, false, false))
    {
        print_tree_visual(ctx, res);
    }

    printf("= ");
    print_tree_inlined(ctx, res, true);
    printf("\n");
}
