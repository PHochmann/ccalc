#include <stdio.h>

#include "simplification.h"
#include "arith_context.h"
#include "../tree/node.h"
#include "../tree/parser.h"
#include "../transformation/rewrite_rule.h"

#define NUM_RULES 5
char *rule_strings[] = { "$x", "x",
    "x+(y+z)", "x+y+z",
    "x*(y*z)", "x*y*z",
    "--x", "x",
    "+x", "x",
};

RewriteRule rules[NUM_RULES];

bool parse_rule(char *before, char *after, RewriteRule *out_rule)
{
    Node *before_n = parse_conveniently(g_ctx, before);
    if (before_n == NULL) return false;
    Node *after_n = parse_conveniently(g_ctx, after);
    if (after_n == NULL) return false;
    *out_rule = get_rule(before_n, after_n);
    return true;
}

bool parse_rules(size_t num_rules, char **input, RewriteRule *out_rules)
{
    for (size_t i = 0; i < num_rules; i++)
    {
        if (!parse_rule(input[2 * i], input[2 * i + 1], &out_rules[i]))
        {
            return false;
        }
    }

    return true;
}

void simplification_init()
{
    parse_rules(NUM_RULES, rule_strings, rules);
}

void simplification_unload()
{
    for (size_t i = 0; i < NUM_RULES; i++)
    {
        free_rule(rules[i]);
    }
}

/*
Summary: Applies all pre-defined rewrite rules to tree
Returns: True when transformations could be applied, False otherwise (currently: only true)
*/
bool core_simplify(Node **tree)
{
    apply_ruleset(tree, NUM_RULES, rules);
    return true;
}
