#include <stdio.h>

#include "../parsing/parser.h"
#include "arith_rules.h"

#define PARSE(n) parse_conveniently(ctx, n)
#define ANS_VAR "ans"

const size_t ARITH_NUM_PREDEFINED_RULES = 8;

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
                //printf("%zu ", j);
                applied_flag = true;
                break;
            }
        }
        if (!applied_flag) return;
    }
}

void arith_init_rules(ParsingContext *ctx)
{
    g_num_rules = ARITH_NUM_PREDEFINED_RULES;
    parse_rules(ctx, ARITH_NUM_PREDEFINED_RULES,
        (char*[]){
            "$x", "x",
            "x+(y+z)", "x+y+z",
            "x*(y*z)", "x*y*z",
            // Derivation
            "name_x'", "1",
            "const_x'", "0",
            "var_x'", "0",
            "(x+y)'", "x'+y'",
            "(x*y)'", "(x'*y)+(x*y')",
        },
        g_rules);
}

// Returns false to indicate math error
bool transform_input(ParsingContext *ctx, Node *tree, bool update_ans)
{
    static Node *ans = NULL; // Constant node that contains result of last evaluation

    // Replace ans
    if (ans != NULL) tree_substitute_var(ctx, tree, ans, ANS_VAR);

    // Apply rules
    apply_ruleset(ctx, tree, g_num_rules, g_rules);

    Matching matching;
    if (find_matching(ctx, tree, PARSE("x'"), &matching))
    {
        printf("Encountered operator with no derivation rule\n");
        free_matching(matching);
        return false;
    }

    // Update ans
    if (update_ans)
    {
        if (ans != NULL) free_tree(ans);
        ans = tree; // Would be safer to copy tree, but is not needed for now
    }

    return true;
}
