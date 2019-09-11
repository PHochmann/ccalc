#include "../engine/parser.h"
#include "arith_rules.h"

const size_t ARITH_NUM_PREDEFINED_RULES = 1;

void arith_reset_rules()
{
    for (size_t i = ARITH_NUM_PREDEFINED_RULES; i < g_num_rules; i++)
    {
        free_rule(g_rules[i]);
    }

    g_num_rules = ARITH_NUM_PREDEFINED_RULES;
}

void arith_init_rules(ParsingContext *ctx)
{
    g_num_rules = ARITH_NUM_PREDEFINED_RULES;

    // $x -> x
    g_rules[0] = get_rule(ctx, parse_conveniently(ctx, "$x"), parse_conveniently(ctx, "x"));
}
