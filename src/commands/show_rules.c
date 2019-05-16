#include <stdio.h>
#include <string.h>

#include "show_rules.h"
#include "command.h"
#include "assignments.h"
#include "util.h"
#include "../engine/constants.h"
#include "../engine/operator.h"
#include "../engine/console_util.h"
#include "../engine/tree_to_string.h"

#define MAX_STRING_LENGTH 300

void show_rules_init()
{

}

bool show_rules_check(char *input)
{
    return strcmp(input, "rules") == 0;
}

void show_rules_exec(ParsingContext *ctx, __attribute__((unused)) char *input)
{
    for (size_t i = 0; i < g_num_rules; i++)
    {
        char l[MAX_STRING_LENGTH];
        char r[MAX_STRING_LENGTH];
        tree_inline(ctx, g_rules[i].before, l, MAX_STRING_LENGTH, true);
        tree_inline(ctx, g_rules[i].after, r, MAX_STRING_LENGTH, true);
        printf("%zu: %s -> %s\n", i, l, r);
    }
}
