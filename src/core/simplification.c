#include <stdio.h>

#include "simplification.h"
#include "arith_context.h"
#include "evaluation.h"
#include "../tree/node.h"
#include "../parsing/parser.h"
#include "../transformation/rewrite_rule.h"

#define NUM_NORMAL_FORM_RULES 4
RewriteRule normal_form_rules[NUM_NORMAL_FORM_RULES];
char *normal_form_strings[] = {
    "$x", "x",
    "--x", "x",
    "x-y", "x+(-y)",
    "-(x+y)", "-x+(-y)",
};

#define NUM_SIMPLIFICATION_RULES 17
RewriteRule simplification_rules[NUM_SIMPLIFICATION_RULES];
char *simplification_strings[] = {
    /* Get a nice sum */
    "x+y", "sum(x,y)",
    "sum([xs], sum([ys]), [zs])", "sum([xs], [ys], [zs])",
    "sum([xs])+sum([ys])", "sum([xs], [ys])",
    "x+sum([xs])", "sum(x, [xs])",
    "sum([xs])+x", "sum([xs], x)",

    /* Get a nice product */
    "x*y", "prod(x,y)",
    "prod([xs], prod([ys]), [zs])", "prod([xs], [ys], [zs])",
    "prod([xs])*prod([ys])", "prod([xs], [ys])",
    "x*prod([xs])", "prod(x, [xs])",
    "prod([xs])*x", "prod([xs], x)",

    /* Simplify sums */
    "sum([xs], 0, [ys])", "sum([xs], [ys])",
    "sum([xs], x, [ys], -x, [zs])", "sum([xs], [ys], [zs])",
    "sum([xs], -x, [ys], x, [zs])", "sum([xs], [ys], [zs])",

    /* Simplify products */
    "prod([xs], 0, [ys])", "0",
    "prod([xs], 1, [ys])", "prod([xs], [ys])",

    /* Operator lift */
    "sum([xs], x, [ys], x, [zs])", "sum([xs], [ys], [zs], 2x)",
    "sum([xs], cA*x, [ys], cB*x, [zs])", "sum([xs], [ys], [zs], (cA+cB)*x)",
};

#define NUM_PRETTY_RULES 6
RewriteRule pretty_rules[NUM_PRETTY_RULES];
char *pretty_strings[] = {
    "x+(y+z)", "x+y+z",
    "x*(y*z)", "x*y*z",
    "sum(x, y)", "x+y",
    "sum(x, [xs])", "x+sum([xs])",
    "--x", "x",
    "+x", "x",
};

bool parse_rule(char *before, char *after, RewriteRule *out_rule)
{
    Node *before_n = parse_conveniently(g_ctx, before);
    if (before_n == NULL) return false;
    Node *after_n = parse_conveniently(g_ctx, after);
    if (after_n == NULL) return false;
    *out_rule = get_rule(&before_n, &after_n);
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

void init_simplification()
{
    parse_rules(NUM_NORMAL_FORM_RULES, normal_form_strings, normal_form_rules);
    parse_rules(NUM_SIMPLIFICATION_RULES, simplification_strings, simplification_rules);
    parse_rules(NUM_PRETTY_RULES, pretty_strings, pretty_rules);
}

void unload_simplification()
{
    for (size_t i = 0; i < NUM_NORMAL_FORM_RULES; i++)
    {
        free_rule(normal_form_rules[i]);
    }
    for (size_t i = 0; i < NUM_SIMPLIFICATION_RULES; i++)
    {
        free_rule(simplification_rules[i]);
    }
    for (size_t i = 0; i < NUM_PRETTY_RULES; i++)
    {
        free_rule(pretty_rules[i]);
    }
}

/*
Summary: Applies all pre-defined rewrite rules to tree
Returns: True when transformations could be applied, False otherwise (currently: only true)
*/
bool core_simplify(Node **tree)
{
    apply_ruleset(tree, NUM_NORMAL_FORM_RULES, normal_form_rules);
    printf("~ ~ ~\n");
    apply_ruleset(tree, NUM_SIMPLIFICATION_RULES, simplification_rules);
    printf("~ ~ ~\n");
    apply_ruleset(tree, NUM_PRETTY_RULES, pretty_rules);
    return true;
}
