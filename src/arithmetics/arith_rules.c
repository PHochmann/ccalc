#include <stdio.h>

#include "arith_rules.h"
#include "arith_context.h"
#include "../parsing/parser.h"
#include "../console_util.h"

#define PARSE(n) parse_conveniently(ctx, n)
#define ANS_VAR "ans"

const size_t ARITH_NUM_PREDEFINED_RULES = 30; // 40

// Functions to add rules at runtime (custom functions)

void arith_reset_rules()
{
    for (size_t i = ARITH_NUM_PREDEFINED_RULES; i < g_num_rules; i++)
    {
        free_rule(g_rules[i]);
    }

    g_num_rules = ARITH_NUM_PREDEFINED_RULES;
}

// - - -

bool parse_rule(ParsingContext *ctx, char *before, char *after, RewriteRule *out_rule)
{
    Node *before_n = parse_conveniently(ctx, before);
    if (before_n == NULL) return false;
    Node *after_n = parse_conveniently(ctx, after);
    if (after_n == NULL) return false;
    *out_rule = get_rule(before_n, after_n);
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

bool is_constant_op(__attribute__((unused)) ParsingContext *ctx, Node *tree)
{
    return tree->type == NTYPE_OPERATOR && tree_count_vars(tree) == 0
        && !find_matching_discarded(ctx, tree, PARSE("x'"));
}

/*
Summary: Tries to apply rules (priorized by order) until no rule can be applied any more
*/
void apply_ruleset(ParsingContext *ctx, Node *tree, size_t num_rules, RewriteRule *rules)
{
    while (true)
    {
        bool applied_flag = false;
        for (size_t j = 0; j < num_rules; j++)
        {
            if (apply_rule(ctx, tree, &rules[j]))
            {
                applied_flag = true;
                break;
            }
        }

        if (!applied_flag) return;
    }
}

// Ideas:
// x+(-x) -> 0

void arith_init_rules(ParsingContext *ctx)
{
    g_num_rules = ARITH_NUM_PREDEFINED_RULES;
    parse_rules(ctx, ARITH_NUM_PREDEFINED_RULES,
        (char*[]){
            "$x", "x",
            // Normal form
            "x+(y+z)", "x+y+z",
            "x*(y*z)", "x*y*z",
            // Operator reduction
            "+x", "x",
            "x%", "x/100",
            "ld(x)", "log(x, 2)",
            "lg(x)", "log(x, 10)",
            "log(x, y)", "ln(x)/ln(y)",
            "exp(x)", "e^x",
            // Simplification
            "--x", "x",
            "0*x", "0",
            "x*0", "0",
            "0+x", "x",
            "x+0", "x",
            "x+x", "2x",
            "x+n*x", "(n+1)*x",
            "x*1", "x",
            "1*x", "x",
            "x*x", "x^2",
            "x*x^n", "x^(n+1)",
            "x/1", "x",
            "0/x", "0",
            "x/x", "1",
            "x*(y/z)", "(x*y)/z",
            "(y/z)*x", "(x*y)/z",
            "(x^y)^z", "x^(y*z)",
            "log(x, x)", "1",
            "x^1", "x",
            "x^0", "1",
            "ln(x^y)", "y*ln(x)",
        },
        g_rules);
}

/*
Summary: Does post-processing of correctly parsed input (i.e. replacing ans and applying RewriteRules)
Returns: Error message or NULL when no error occurred
*/
char *transform_input(ParsingContext *ctx, Node *tree, bool update_ans)
{
    static Node *ans = NULL; // Constant node that contains result of last evaluation

    // Replace ans
    if (ans != NULL) tree_substitute_var(ctx, tree, ans, ANS_VAR);

    // Apply rules
    apply_ruleset(ctx, tree, g_num_rules, g_rules);

    // Check for remaining derivation operators
    if (find_matching_discarded(ctx, tree, PARSE("x'")))
    {
        return "Derivation currently not implemented";
    }

    // Update ans
    if (update_ans)
    {
        if (ans != NULL) free_tree(ans);
        ans = tree; // Would be safer to copy tree, but it is not needed for now
    }

    return NULL;
}
