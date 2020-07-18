#include <math.h>

#include "../tree/tree_util.h"
#include "../tree/tree_to_string.h"
#include "../parsing/parser.h"
#include "../transformation/rewrite_rule.h"
#include "simplification.h"
#include "arith_context.h"
#include "evaluation.h"
#include "../util/console_util.h"

#define P(x) parse_conveniently(g_ctx, x)

Node *deriv_before;
Node *deriv_after;
Node *malformed_derivA;
Node *malformed_derivB;

#define RULESETS_PATH "rulesets.rules"
#define NUM_RULESETS 5
Vector rulesets[NUM_RULESETS];

void replace_negative_consts(Node **tree)
{
    if (get_type(*tree) == NTYPE_CONSTANT)
    {
        if (get_const_value(*tree) < 0)
        {
            Node *minus_op = malloc_operator_node(ctx_lookup_op(g_ctx, "-", OP_PLACE_PREFIX), 1);
            set_child(minus_op, 0, malloc_constant_node(fabs(get_const_value(*tree))));
            tree_replace(tree, minus_op);
        }
    }
    else
    {
        if (get_type(*tree) == NTYPE_OPERATOR)
        {
            for (size_t i = 0; i < get_num_children(*tree); i++)
            {
                replace_negative_consts(get_child_addr(*tree, i));
            }
        }
    }
}

void init_simplification()
{
    deriv_before = P("x'");
    deriv_after = P("deriv(x, z)");
    malformed_derivA = P("deriv(x, cX)");
    malformed_derivB = P("deriv(x, oX)");
    __attribute__((unused)) size_t num_rulesets;
    parse_rulesets(RULESETS_PATH, g_ctx, NUM_RULESETS, &num_rulesets, rulesets);
}

void unload_simplification()
{
    free_tree(deriv_before);
    free_tree(deriv_after);
    free_tree(malformed_derivA);
    free_tree(malformed_derivB);
    for (size_t i = 0; i < NUM_RULESETS; i++)
    {
        free_ruleset(&rulesets[i]);
    }
}

/*
Summary: Applies all pre-defined rewrite rules to tree
Returns: True when transformations could be applied, False otherwise
*/
bool core_simplify(Node **tree)
{
    Matching matching;
    Node **matched;
    while ((matched = find_matching(tree, deriv_before, &matching, NULL)) != NULL)
    {
        char *var_name;
        size_t var_count = count_variables_distinct(*tree);

        if (var_count > 1)
        {
            report_error("You can only use expr' when there is not more than one variable in expr.\n");
            return false;
        }

        Node *replacement = tree_copy(deriv_after);
        if (var_count == 1)
        {
            list_variables(*tree, &var_name);
            tree_replace(get_child_addr(replacement, 1), P(var_name));
        }
        tree_replace(get_child_addr(replacement, 0), tree_copy(matching.mapped_nodes[0].nodes[0]));
        tree_replace(matched, replacement);
    }

    if (find_matching_discarded(*tree, malformed_derivA, NULL) != NULL
        || find_matching_discarded(*tree, malformed_derivA, NULL) != NULL)
    {
        report_error("Second operand of deriv must be variable.\n");
        return false;
    }

    // Apply elimination rules
    apply_ruleset(tree, &rulesets[0]);

    Node *tree_before = NULL;
    do
    {
        free_tree(tree_before);
        tree_before = tree_copy(*tree);

        /*for (size_t j = 0; j < rulesets[0].num_rules; j++)
        {
            if (apply_rule(tree, &rulesets[0].rules[j]))
            {
                break;
            }
        }*/

        apply_ruleset(tree, &rulesets[2]); // Normal form rules
        apply_ruleset(tree, &rulesets[3]); // Simplification rules
        replace_constant_subtrees(tree, false, op_evaluate);
        apply_ruleset(tree, &rulesets[4]); // Pretty rules
        replace_constant_subtrees(tree, false, op_evaluate);
        replace_negative_consts(tree);
    } while (tree_compare(tree_before, *tree) != NULL);
    free_tree(tree_before);

    if (find_matching_discarded(*tree, deriv_after, NULL) != NULL)
    {
        report_error("Could not derivate expression.\n");
        return false;
    }

    return true;
}
