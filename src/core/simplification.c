#define _GNU_SOURCE
#include <math.h>
#include <string.h>
#include <stdio.h>

#include "../tree/tree_util.h"
#include "../tree/tree_to_string.h"
#include "../parsing/parser.h"
#include "../transformation/rewrite_rule.h"
#include "simplification.h"
#include "arith_context.h"
#include "evaluation.h"
#include "rules.h"
#include "../util/console_util.h"

#define P(x) parse_conveniently(g_ctx, x)

Node *deriv_before;
Node *deriv_after;
Node *malformed_derivA;
Node *malformed_derivB;

#define NUM_RULESETS 5
Vector rulesets[NUM_RULESETS];

bool exponent_even_filter(char *var, NodeList nodes)
{
    if (strcmp(var, "y") != 0) return true;

    // The following should not be needed to the nature of the matching algorithm
    // Be brave and dereference!
    //if (nodes.size != 1) return false;

    if (count_variables(nodes.nodes[0], false) == 0)
    {
        double res = arith_evaluate(nodes.nodes[0]);
        if (res != (int)res) return false;
        if ((int)res % 2 == 0)
        {
            return true;
        }
    }
    return false;
}

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

    for (size_t i = 0; i < NUM_RULESETS; i++)
    {
        rulesets[i] = get_empty_ruleset();
    }

    __attribute__((unused)) size_t num_rulesets;
    FILE *stream = fmemopen(reduction_strings, strlen(reduction_strings), "r");
    parse_rulesets(stream, g_ctx, prefix_filter, 1, &num_rulesets, rulesets);
    fclose(stream);
    stream = fmemopen(derivation_strings, strlen(derivation_strings), "r");
    parse_rulesets(stream, g_ctx, prefix_filter, 1, &num_rulesets, rulesets + 1);
    fclose(stream);
    stream = fmemopen(normal_form_strings, strlen(normal_form_strings), "r");
    parse_rulesets(stream, g_ctx, prefix_filter, 1, &num_rulesets, rulesets + 2);
    fclose(stream);
    stream = fmemopen(simplification_strings, strlen(simplification_strings), "r");
    parse_rulesets(stream, g_ctx, prefix_filter, 1, &num_rulesets, rulesets + 3);
    fclose(stream);
    stream = fmemopen(pretty_strings, strlen(pretty_strings), "r");
    parse_rulesets(stream, g_ctx, prefix_filter, 1, &num_rulesets, rulesets + 4);
    fclose(stream);
    add_to_ruleset(&rulesets[3], get_rule(P("(-x)^y"), P("x^y"), exponent_even_filter));
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
