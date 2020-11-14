#include <math.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "../../engine/tree/tree_util.h"
#include "../../engine/tree/tree_to_string.h"
#include "../../engine/transformation/rewrite_rule.h"
#include "../../engine/transformation/rule_parsing.h"
#include "../../engine/parsing/parser.h"
#include "../../engine/util/console_util.h"
#include "../../engine/util/linked_list.h"

#include "../core/arith_context.h"
#include "../core/arith_evaluation.h"
#include "simplification.h"
#include "propositional_context.h"
#include "propositional_evaluation.h"

#define P(x) parse_conveniently(g_ctx, x)

#define RULESET_FILENAME "rules.rr"
#define ITERATION_CAP    1000
#define NUM_RULESETS     7

bool initialized = false;
Vector rulesets[NUM_RULESETS];

Pattern deriv_before;
Node *deriv_after;
Pattern malformed_deriv;

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
    if (access(RULESET_FILENAME, R_OK) == -1)
    {
        report_error("Simplification deactivated since ruleset not found or readable.\n");
        return;
    }

    init_propositional_ctx();
    FILE *ruleset_file = fopen(RULESET_FILENAME, "r");
    for (size_t i = 0; i < NUM_RULESETS; i++)
    {
        rulesets[i] = get_empty_ruleset();
    }
    if (parse_rulesets_from_file(ruleset_file, g_ctx, g_propositional_ctx, NUM_RULESETS, rulesets) != NUM_RULESETS)
    {
        report_error("Too few simplification rulesets defined in %s.\n", RULESET_FILENAME);
    }
    fclose(ruleset_file);

    deriv_before = get_pattern(P("x'"), 0, NULL);
    deriv_after = P("deriv(x, z)");
    if (!parse_pattern("deriv(x, y) WHERE !(type(cx) == VAR)", g_ctx, g_propositional_ctx, &malformed_deriv))
    {
        software_defect("malformed deriv pattern parser error\n");
    }

    initialized = true;
}

void unload_simplification()
{
    free_pattern(&deriv_before);
    free_tree(deriv_after);
    free_pattern(&malformed_deriv);
    for (size_t i = 0; i < NUM_RULESETS; i++)
    {
        free_ruleset(&rulesets[i]);
    }
    unload_propositional_ctx();
    initialized = false;
}

/*
Summary: Applies all pre-defined rewrite rules to tree
Returns: True when transformations could be applied, False otherwise
*/
bool core_simplify(Node **tree)
{
    if (!initialized) return true;

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
    apply_ruleset(tree, &rulesets[0], propositional_checker, ITERATION_CAP);

    Matching matching;
    Node **matched;
    while ((matched = find_matching((const Node**)tree, &deriv_before, NULL, &matching)) != NULL)
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

    if (does_match(*tree, &malformed_deriv, propositional_checker))
    {
        report_error("Second operand of function 'deriv' must be variable.\n");
        return false;
    }

    // Eliminiate deriv(x,y)
    // Since the deriv-ruleset is extremely expansive in nature,
    // aggressively replace constant subtrees
    VectorIterator deriv_it = vec_get_iterator(&rulesets[1]);
    while (apply_ruleset_by_iterator(tree, (Iterator*)&deriv_it, propositional_checker, 1) != 0)
    {
        iterator_reset((Iterator*)&deriv_it);
        tree_reduce_constant_subtrees(tree, arith_op_evaluate);
    };

    // If the tree still contains deriv-operators, the user attempted to derivate 
    // a subtree for which no reduction rule exists.
    Node **unresolved_derivation = find_op((const Node**)tree, get_op(deriv_after));
    if (unresolved_derivation != NULL)
    {
        if (get_type(get_child(*unresolved_derivation, 0)) != NTYPE_OPERATOR)
        {
            software_defect("Derivation failed.\n");
        }
        else
        {
            report_error("Can't derivate operator '%s'.\n", get_op(get_child(*unresolved_derivation, 0))->name);
        }
        return false;
    }

    tree_reduce_constant_subtrees(tree, arith_op_evaluate);
    replace_negative_consts(tree);

    //printf("\n\nBeginning with flattening...\n");

    apply_ruleset(tree, &rulesets[2], propositional_checker, SIZE_MAX); // Normal form rules
    tree_reduce_constant_subtrees(tree, arith_op_evaluate);

    //printf("\n\nBeginning with main simplification...\n");

    // Simplification - don't hang forever
    size_t simp_cap = 100;
    while (apply_ruleset(tree, &rulesets[3], propositional_checker, 10) != 0 && simp_cap-- != 0)
    {
        tree_reduce_constant_subtrees(tree, arith_op_evaluate);
    }
    replace_negative_consts(tree);

    //printf("\n\nBeginning with folding...\n");

    apply_ruleset(tree, &rulesets[4], propositional_checker, SIZE_MAX); // Remove sum() and prod()
    tree_reduce_constant_subtrees(tree, arith_op_evaluate);
    replace_negative_consts(tree);

    //printf("\n\nBeginning with pretty printing...\n");

    apply_ruleset(tree, &rulesets[5], propositional_checker, SIZE_MAX); // Pretty printing
    tree_reduce_constant_subtrees(tree, arith_op_evaluate);

    //printf("\n\nBeginning with ordering...\n");

    apply_ruleset(tree, &rulesets[6], propositional_checker, SIZE_MAX); // Ordering
    tree_reduce_constant_subtrees(tree, arith_op_evaluate);
    replace_negative_consts(tree);

    return true;
}