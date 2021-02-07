#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "../../util/console_util.h"
#include "../../engine/tree/tree_to_string.h"
#include "../../engine/tree/tree_util.h"

#include "cmd_evaluation.h"
#include "../core/arith_context.h"
#include "../core/history.h"
#include "../core/arith_evaluation.h"

#define ERROR_FMT "Error: %s\n"

int cmd_evaluation_check(__attribute__((unused)) const char *input)
{
    return true;
}

/*
Summary: The evaluation command is executed when input is no other command (hence last in command array in commands.c)
*/
bool cmd_evaluation_exec(char *input, __attribute__((unused)) int code)
{
        Node *node;
        if (!arith_parse(input, ERROR_FMT, 0, &node)) return false;
        printf("= ");
        print_tree(node, true);
        printf("\n");
        free_tree(node);
        return true;
}
