#include <stdio.h>
#include <string.h>

#include "show_rules.h"
#include "command.h"
#include "assignments.h"
#include "../engine/constants.h"
#include "../engine/operator.h"
#include "../engine/string_util.h"

#define MAX_STRING_LENGTH 200

void show_rules_init()
{

}

bool show_rules_check(char *input)
{
    return strcmp(input, "rules") == 0;
}

void print_rule(ParsingContext *ctx, RewriteRule *rule)
{
    char l[MAX_STRING_LENGTH];
    char r[MAX_STRING_LENGTH];
    tree_inline(ctx, rule->before, l, MAX_STRING_LENGTH, true);
    tree_inline(ctx, rule->after, r, MAX_STRING_LENGTH, true);
    printf("%s -> %s", l, r);
}

void show_rules_exec(ParsingContext *ctx, __attribute__((unused)) char *input)
{
    if (g_num_rules == 0)
    {
        printf("No rules defined.\n");
        return;
    }

    for (size_t i = 0; i < g_num_rules; i++)
    {
        printf("%zu: ", i + 1);
        print_rule(ctx, &g_rules[i]);
        printf("\n");
    }
}
