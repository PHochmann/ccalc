#include <math.h>
#include <string.h>
#include <stdio.h>

#include "../tree/tree_util.h"
#include "../tree/tree_to_string.h"
#include "../transformation/rewrite_rule.h"
#include "../parsing/parser.h"
#include "../util/console_util.h"
#include "../util/linked_list.h"
#include "simplification.h"
#include "arith_context.h"
#include "evaluation.h"
#include "rules.h"
#include "filters.h"

#define P(x) parse_conveniently(g_ctx, x)

#define CAP             1000 // To protect against endless loops
#define NUM_DONT_REDUCE 4
const Operator *dont_reduce[NUM_DONT_REDUCE];

Node *deriv_before;
Node *deriv_after;
Node *malformed_derivA;
Node *malformed_derivB;

// Contains parsed and compiled rules
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
    for (size_t i = 0; i < NUM_RULESETS; i++)
    {
        rulesets[i] = get_empty_ruleset();
        parse_ruleset_from_string(g_rulestrings[i], g_ctx, prefix_filter, rulesets + i);
    }

    // Add rules with special filters
    add_to_ruleset(&rulesets[1], get_rule(P("deriv(x,y)"), P("0"), constant_derivative_filter));
    add_to_ruleset(&rulesets[3], get_rule(P("(-x)^y"), P("x^y"), exponent_even_filter));

    deriv_before = P("x'");
    deriv_after = P("deriv(x, z)");
    malformed_derivA = P("deriv(x, cX)");
    malformed_derivB = P("deriv(x, oX)");
    preprocess_pattern(deriv_before);
    preprocess_pattern(deriv_after);
    preprocess_pattern(malformed_derivA);
    preprocess_pattern(malformed_derivB);
    dont_reduce[0] = ctx_lookup_op(g_ctx, "pi", OP_PLACE_FUNCTION);
    dont_reduce[1] = ctx_lookup_op(g_ctx, "e", OP_PLACE_FUNCTION);
    dont_reduce[2] = ctx_lookup_op(g_ctx, "rand", OP_PLACE_FUNCTION);
    dont_reduce[3] = ctx_lookup_op(g_ctx, "$", OP_PLACE_PREFIX); // There are no evaluation cases before +
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
    // Replace avg(x,y,z...) by sum(x,y,z...)/num_children
    Node **avg_node;
    while ((avg_node = find_op((const Node**)tree, ctx_lookup_op(g_ctx, "avg", OP_PLACE_FUNCTION))) != NULL)
    {
        set_op(*avg_node, ctx_lookup_op(g_ctx, "sum", OP_PLACE_FUNCTION));
        Node *replacement = malloc_operator_node(ctx_lookup_op(g_ctx, "/", OP_PLACE_INFIX), 2);
        set_child(replacement, 0, *avg_node);
        set_child(replacement, 1, malloc_constant_node(get_num_children(*avg_node)));
        // Now some careful pointer bending from one tree to the other
        // Note: Usually, pointer spaghetti is discouraged
        for (size_t i = 0; i < get_num_children(*avg_node); i++)
        {
            set_child(get_child(replacement, 0), i, get_child(*avg_node, i));
        }
        *avg_node = replacement;
    }

    // Apply elimination rules
    apply_ruleset(tree, &rulesets[0], CAP);

    Matching matching;
    Node **matched;
    while ((matched = find_matching((const Node**)tree, deriv_before, &matching, NULL)) != NULL)
    {
        // Check if there is more than one variable in within derivative shorthand
        const char *vars[2];
        size_t var_count = list_variables(*matched, 2, vars);
        if (var_count > 1)
        {
            report_error("You can only use expr' when there is not more than one variable in expr.\n");
            return false;
        }

        Node *replacement = tree_copy(deriv_after);
        if (var_count == 1)
        {
            tree_replace(get_child_addr(replacement, 1), malloc_variable_node(vars[0], 0));
        }
        tree_replace(get_child_addr(replacement, 0), tree_copy(matching.mapped_nodes[0].nodes[0]));
        tree_replace(matched, replacement);
    }

    if (does_match(*tree, malformed_derivA, prefix_filter)
        || does_match(*tree, malformed_derivB, prefix_filter))
    {
        report_error("Second operand of function 'deriv' must be variable.\n");
        return false;
    }

    // Eliminiate deriv(x,y)
    // Since the deriv-ruleset is extremely expansive in nature,
    // aggressively replace constant subtrees
    VectorIterator deriv_it = vec_get_iterator(&rulesets[1]);
    while (apply_ruleset_by_iterator(tree, (Iterator*)&deriv_it, 1) != 0)
    {
        iterator_reset((Iterator*)&deriv_it);
        replace_constant_subtrees(tree, op_evaluate, NUM_DONT_REDUCE, dont_reduce);
    };

    // If the tree still contains deriv-operators, the user attempted to derivate 
    // a subtree for which no reduction rule exists.
    Node **unresolved_derivation = find_op((const Node**)tree, get_op(deriv_after));
    if (unresolved_derivation != NULL)
    {
        report_error("Can't derivate operator '%s'.\n", get_op(get_child((*unresolved_derivation), 0))->name);
        return false;
    }

    replace_constant_subtrees(tree, op_evaluate, NUM_DONT_REDUCE, dont_reduce);
    replace_negative_consts(tree);

    apply_ruleset(tree, &rulesets[2], SIZE_MAX); // Normal form rules
    replace_constant_subtrees(tree, op_evaluate, NUM_DONT_REDUCE, dont_reduce);

    while (apply_ruleset(tree, &rulesets[3], 10) != 0)
    {
        replace_constant_subtrees(tree, op_evaluate, NUM_DONT_REDUCE, dont_reduce);
    }; // Simplification

    apply_ruleset(tree, &rulesets[4], SIZE_MAX); // Remove sum() and prod()
    replace_constant_subtrees(tree, op_evaluate, NUM_DONT_REDUCE, dont_reduce);
    replace_negative_consts(tree);

    #ifdef DEBUG
    printf("Beginning with pretty printing...\n");
    #endif

    apply_ruleset(tree, &rulesets[5], SIZE_MAX); // Pretty printing
    replace_constant_subtrees(tree, op_evaluate, NUM_DONT_REDUCE, dont_reduce);
    replace_negative_consts(tree);

    return true;
}
