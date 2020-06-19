#include <stdio.h>

#include "simplification.h"
#include "arith_context.h"
#include "evaluation.h"
#include "../tree/node.h"
#include "../parsing/parser.h"
#include "../transformation/rewrite_rule.h"

#define NUM_NORMAL_FORM_RULES 8
RewriteRule normal_form_rules[NUM_NORMAL_FORM_RULES];
char *normal_form_strings[] = {
    "vx+cx", "cx+vx",
    "vx*cx", "cx*vx",
    "x+(y+z)", "x+y+z",
    "x*(y*z)", "x*y*z",

    "x-y", "x+(-y)",
    "$x", "x",
    "--x", "x",
    "-(x+y)", "-x+(-y)",
    "-(x*y)", "(-x)*y"
};

#define NUM_SIMPLIFICATION_RULES 28
RewriteRule simplification_rules[NUM_SIMPLIFICATION_RULES];
char *simplification_strings[] = {

    /* Move constants */
    "sum([xs], vx, [ys], cx, [zs])", "sum(cx, [xs], vx, [ys], [zs])", 

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
    "sum(x)", "x",
    "sum([xs], 0, [ys])", "sum([xs], [ys])",
    "sum([xs], x, [ys], -x, [zs])", "sum([xs], [ys], [zs])",
    "sum([xs], -x, [ys], x, [zs])", "sum([xs], [ys], [zs])",

    /* Simplify products */
    "prod(x)", "x",
    "prod([xs], 0, [ys])", "0",
    "prod([xs], 1, [ys])", "prod([xs], [ys])",
    "prod([xs], -1, [ys])", "-prod([xs], [ys])",

    /* Products within sum */
    "sum([xs], prod(a, x), [ys], x, [zs])", "sum([xs], prod(a+1, x), [ys], [zs])",
    "sum([xs], prod(x, a), [ys], x, [zs])", "sum([xs], prod(x, a+1), [ys], [zs])",
    "sum([xs], prod(b, x), [ys], prod(a, x), [zs])", "sum([xs], [ys], prod(a+b, x), [zs])",
    "sum([xs], prod(x, b), [ys], prod(x, a), [zs])", "sum([xs], [ys], prod(a+b, x), [zs])",
    "sum([xs], prod(b, x), [ys], prod(x, a), [zs])", "sum([xs], [ys], prod(a+b, x), [zs])",
    "sum([xs], prod(x, b), [ys], prod(a, x), [zs])", "sum([xs], [ys], prod(a+b, x), [zs])",
    "sum([xs], x, [ys], prod(a, x), [zs])", "sum([xs], [ys], prod(a+1, x), [zs])",
    "sum([xs], x, [ys], prod(x, a), [zs])", "sum([xs], [ys], prod(x, a+1), [zs])",
    "sum([xs], x, [ys], x, [zs])", "sum(prod(2, x), [xs], [ys], [zs])",
};

#define NUM_PRETTY_RULES 9
RewriteRule pretty_rules[NUM_PRETTY_RULES];
char *pretty_strings[] = {
    "x+(y+z)", "x+y+z",
    "x*(y*z)", "x*y*z",
    "sum(x, y)", "x+y",
    "sum(x, [xs])", "x+sum([xs])",
    "sum(x)", "x",
    "prod(x)", "x",
    "prod(x, y)", "x*y",
    "prod(x, [xs])", "x*sum([xs])",
    "--x", "x",
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
