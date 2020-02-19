#include <stdio.h>

#include "arith_transformation.h"
#include "arith_context.h"
#include "../tree/node.h"
#include "../tree/parser.h"

#define ANS_VAR        "ans"
#define ANS_HISTORY_SIZE 10

#define ARITH_NUM_RULES 5
#define ARITH_MAX_RULES (ARITH_NUM_RULES + 10)

size_t num_rules;
RewriteRule rules[ARITH_MAX_RULES];

size_t next_ans;
Node *ans[ANS_HISTORY_SIZE]; // Results of last evaluations (flattened to single ConstantNodes)
Node *ans_pattern;

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
void apply_ruleset(Node **tree, size_t num_rules, RewriteRule *rules)
{
    while (true)
    {
        bool applied_flag = false;
        for (size_t j = 0; j < num_rules; j++)
        {
            if (apply_rule(tree, &rules[j]))
            {
                applied_flag = true;
                break;
            }
        }

        if (!applied_flag) return;
    }
}

/*
Params
    index: 0 -> last evaluation, 1 -> second last evaluation etc.
*/
Node *get_ans(size_t index)
{
    return ans[((int)next_ans - 1 - index) % ANS_HISTORY_SIZE];
}

// ~ ~ ~ ~ ~ ~ End of helper functions, public functions follow ~ ~ ~ ~ ~ ~

void arith_unload_transformation()
{
    for (size_t i = 0; i < num_rules; i++)
    {
        free_rule(rules[i]);
    }
    num_rules = ARITH_NUM_RULES;

    for (size_t i = 0; i < ANS_HISTORY_SIZE; i++)
    {
        if (ans[i] != NULL)
        {
            free_tree(ans[i]);
            ans[i] = NULL;
        }
    }
    free_tree(ans_pattern);
}

void arith_init_transformation()
{
    num_rules = ARITH_NUM_RULES;
    parse_rules(ARITH_NUM_RULES,
        (char*[]){ "$x", "x",
            "x+(y+z)", "x+y+z",
            "x*(y*z)", "x*y*z",
            "--x", "x",
            "+x", "x",
        },
        rules);

    next_ans = 0;
    ans_pattern = parse_conveniently(g_ctx, "@x");
}

bool arith_can_add_rule()
{
    return num_rules != ARITH_MAX_RULES;
}

size_t arith_get_num_userdefined()
{
    return num_rules - ARITH_NUM_RULES;
}

RewriteRule *arith_get_userdefined(size_t index)
{
    return &rules[ARITH_NUM_RULES + index];
}

bool arith_add_rule(RewriteRule rule)
{
    if (!arith_can_add_rule())
    {
        return false;
    }
    else
    {
        rules[num_rules] = rule;
        num_rules++;
        return true;
    }
}

bool arith_pop_rule()
{
    if (arith_get_num_userdefined() == 0)
    {
        return false;
    }
    else
    {
        free_rule(rules[num_rules - 1]);
        num_rules--;
        return true;
    }
}

void arith_update_ans(ConstantType value)
{
    if (ans[next_ans] != NULL) free_tree(ans[next_ans]);
    ans[next_ans] = malloc_constant_node(value);
    next_ans = (next_ans + 1) % ANS_HISTORY_SIZE;
}

/*
Summary: Does post-processing of correctly parsed input (i.e. replacing ans and applying rewrite rules)
*/
bool arith_transform_input(bool all_rules, Node **tree)
{
    if (all_rules)
    {
        apply_ruleset(tree, num_rules, rules);
    }
    else
    {
        // Even when all_rules is set to false, we want to execute predefined rules (e.g. eliminiation of $)
        apply_ruleset(tree, ARITH_NUM_RULES, rules);
    }

    // Replace ans@x
    Matching ans_matching;
    while (find_matching(tree, ans_pattern, &ans_matching))
    {
        if (count_variables(ans_matching.mapped_nodes[0]) > 0)
        {
            printf("Error: @-expression must be constant.\n");
            free_matching(ans_matching);
            return false;
        }

        int index = (int)arith_eval(ans_matching.mapped_nodes[0]);

        if (index < 0 || index >= ANS_HISTORY_SIZE)
        {
            printf("Error: @x for x between 0 and %d.\n", ANS_HISTORY_SIZE - 1);
            free_matching(ans_matching);
            return false;
        }
        if (get_ans(index) == 0)
        {
            printf("Error: This part of the history is not set yet.\n");
            free_matching(ans_matching);
            return false;
        }

        tree_replace(ans_matching.matched_tree, tree_copy(get_ans(index)));
        free_matching(ans_matching);
    }

    // Replace normal ans
    if (get_ans(0) != NULL)
    {
        replace_variable_nodes(tree, get_ans(0), ANS_VAR);
    }
    else
    {
        if (count_variable_nodes(*tree, ANS_VAR) > 0)
        {
            printf("Error: This part of the history is not set yet.\n");
            return false;
        }
    }

    return true;
}
