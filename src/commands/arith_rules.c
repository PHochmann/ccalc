#include "../engine/parser.h"
#include "arith_rules.h"
#include "console_util.h"

const size_t ARITH_NUM_PREDEFINED_RULES = 9;

void arith_reset_rules()
{
    for (size_t i = ARITH_NUM_PREDEFINED_RULES; i < g_num_rules; i++)
    {
        free_rule(g_rules[i]);
    }

    g_num_rules = ARITH_NUM_PREDEFINED_RULES;
}

bool parse_rule(ParsingContext *ctx, char *before, char *after, RewriteRule *out_rule)
{
    Node *before_n = parse_conveniently(ctx, before);
    if (before_n == NULL) return false;
    Node *after_n = parse_conveniently(ctx, after);
    if (after_n == NULL) return false;
    *out_rule = get_rule(ctx, before_n, after_n);
    return true;
}

bool parse_rules(ParsingContext *ctx, size_t num_rules, char **input, RewriteRule *out_rules)
{
    for (size_t i = 0; i < num_rules; i++)
    {
        if (!parse_rule(ctx, input[2 * i], input[2 * i + 1], &out_rules[i]))
        {
            return false;
        }
    }

    return true;
}

void infer(RewriteRule *a, RewriteRule *b)
{
    printf("[");
    print_tree_inlined(a->context, a->before, true);
    printf(" -> ");
    print_tree_inlined(a->context, a->after, true);
    printf("] AND [");
    print_tree_inlined(b->context, b->before, true);
    printf(" -> ");
    print_tree_inlined(b->context, b->after, true);
    printf("] YIELDS ");

    RewriteRule c;

    if (infer_rule(a->context, a, b, &c))
    {
        printf("[");
        print_tree_inlined(c.context, c.before, true);
        printf(" -> ");
        print_tree_inlined(c.context, c.after, true);
        printf("]\n");
    }
    else
    {
        printf("NOTHING\n");
    }
}

void arith_init_rules(ParsingContext *ctx)
{
    g_num_rules = ARITH_NUM_PREDEFINED_RULES;
    parse_rules(ctx, ARITH_NUM_PREDEFINED_RULES,
        (char*[]){
            "$x", "x",
            "x+0", "x",
            "--x", "x",
            "-x+y", "y-x",
            "x+(-y)", "x-y",
            "x-x", "0",
            "x-(-y)", "x+y",
            "x+x", "2x",
            "x*y+y", "(x+1)y"
        },
        g_rules);

    // Experimental: Try to infer 0+x
    RewriteRule commutativity;
    parse_rule(ctx, "x+y", "y+x", &commutativity);
    infer(&commutativity, &g_rules[1]);
    infer(&commutativity, &g_rules[2]);
}
