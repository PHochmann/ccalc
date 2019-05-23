#include <stdio.h>
#include <string.h>

#include "show_rules.h"
#include "command.h"
#include "assignments.h"
#include "util.h"
#include "../engine/constants.h"
#include "../engine/operator.h"
#include "../engine/string_util.h"

#define MAX_STRING_LENGTH 300

void show_rules_init()
{

}

bool show_rules_check(char *input)
{
    return (strcmp(input, "rules") == 0) || (strcmp(input, "rules pop") == 0);
}

void print_rule(ParsingContext *ctx, RewriteRule *rule)
{
    char l[MAX_STRING_LENGTH];
    char r[MAX_STRING_LENGTH];
    tree_inline(ctx, rule->before, l, MAX_STRING_LENGTH, true);
    tree_inline(ctx, rule->after, r, MAX_STRING_LENGTH, true);
    printf("%s -> %s", l, r);
}

void show_rules_exec(ParsingContext *ctx, char *input)
{
    if (strcmp(input, "rules pop") == 0)
    {
        if (g_num_rules != 0)
        {
            printf("Popped ");
            print_rule(ctx, &g_rules[g_num_rules - 1]);
            printf("\n");
            g_num_rules--;
        }
        else
        {
            printf("There is not rule to pop.\n");
        }
    }
    else // Only show rules, input was "rules"
    {
        if (g_num_rules == 0)
        {
            printf("No rules defined.\n");
            return;
        }

        for (size_t i = 0; i < g_num_rules; i++)
        {
            printf("%zu: ", i);
            print_rule(ctx, &g_rules[i]);
            printf("\n");
        }
    }
}
