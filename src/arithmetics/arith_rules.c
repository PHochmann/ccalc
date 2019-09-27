#include <stdio.h>

#include "arith_rules.h"
#include "arith_context.h"
#include "../parsing/parser.h"
#include "../console_util.h"

#define PARSE(n) parse_conveniently(n)
#define ANS_VAR "ans"

// Functions to add rules at runtime (custom functions)

void arith_reset_rules()
{
    for (size_t i = ARITH_NUM_RULES; i < g_num_rules; i++)
    {
        free_rule(g_rules[i]);
    }

    g_num_rules = ARITH_NUM_RULES;
}

// - - -

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

/*
Summary: Tries to apply rules (priorized by order) until no rule can be applied any more
*/
void apply_ruleset(Node *tree, size_t num_rules, RewriteRule *rules)
{
    while (true)
    {
        bool applied_flag = false;
        for (size_t j = 0; j < num_rules; j++)
        {
            if (apply_rule(g_ctx, tree, &rules[j]))
            {
                applied_flag = true;
                break;
            }
        }

        if (!applied_flag) return;
    }
}

void arith_init_rules()
{
    g_num_rules = ARITH_NUM_RULES;
    parse_rules(ARITH_NUM_RULES,
        (char*[]){
            "$x", "x",
        },
        g_rules);
}

/*
Summary: Does post-processing of correctly parsed input (i.e. replacing ans and applying RewriteRules)
Returns: Error message or NULL when no error occurred
*/
void transform_input(Node *tree, bool update_ans)
{
    static Node *ans = NULL; // Constant node that contains result of last evaluation

    // Replace ans
    if (ans != NULL) tree_substitute_var(g_ctx, tree, ans, ANS_VAR);

    // Apply rules
    apply_ruleset(tree, g_num_rules, g_rules);

    // Update ans
    if (update_ans)
    {
        if (ans != NULL) free_tree(ans);
        ans = tree; // Would be safer to copy tree, but it is not needed for now
    }
}
