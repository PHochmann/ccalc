#include <stdio.h>
#include <string.h>

#include "show_rules.h"
#include "arith_context.h"
#include "arith_rules.h"
#include "assignments.h"
#include "../engine/operator.h"
#include "../engine/string_util.h"

static const size_t MAX_STRING_LENGTH = 200;

void show_rules_init() { }

bool show_rules_check(char *input)
{
    return strcmp(input, "rules") == 0 || strcmp(input, "rules clear") == 0;
}

void print_rule(ParsingContext *ctx, RewriteRule *rule)
{
    char l[MAX_STRING_LENGTH];
    char r[MAX_STRING_LENGTH];
    tree_inline(ctx, rule->before, l, MAX_STRING_LENGTH, true);
    tree_inline(ctx, rule->after, r, MAX_STRING_LENGTH, true);
    printf("%s -> %s", l, r);
}

/*
Summary: Prints a string representation for currently defined rules
*/
void show_rules_exec(ParsingContext *ctx, char *input)
{
    if (strcmp(input, "rules") == 0)
    {
        if (g_num_rules == ARITH_NUM_PREDEFINED_RULES)
        {
            printf("No rules defined.\n");
            return;
        }

        for (size_t i = ARITH_NUM_PREDEFINED_RULES; i < g_num_rules; i++)
        {
            printf("%zu: ", i + 1);
            print_rule(ctx, &g_rules[i]);
            printf("\n");
        }
    }
    else // input must be "rules clear"
    {
        arith_reset_ctx(); // To remove user-defined functions from parsing context
        arith_reset_rules(); // To remove any user-defined rules
        printf("Rules cleared.\n");
    }
}
