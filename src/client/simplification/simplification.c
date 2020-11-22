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
#define NUM_RULESETS 7

bool initialized = false;
Vector rulesets[NUM_RULESETS];

Pattern deriv_before;
Node *deriv_after;
Pattern malformed_deriv;

static void replace_negative_consts(Node **tree)
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

ssize_t init_simplification(char *file)
{
    if (access(file, R_OK) == -1) return -1;

    FILE *ruleset_file = fopen(file, "r");
    for (size_t i = 0; i < NUM_RULESETS; i++)
    {
        rulesets[i] = get_empty_ruleset();
    }
    if (parse_rulesets_from_file(ruleset_file, g_ctx, g_propositional_ctx, NUM_RULESETS, rulesets) != NUM_RULESETS)
    {
        report_error("Too few simplification rulesets defined in %s.\n", file);
        return -1;
    }
    fclose(ruleset_file);

    deriv_before = get_pattern(P("x'"), 0, NULL);
    deriv_after = P("deriv(x, z)");
    parse_pattern("deriv(x, y) WHERE !(type(cx) == VAR)", g_ctx, g_propositional_ctx, &malformed_deriv);

    initialized = true;

    ssize_t res = 0;
    for (size_t i = 0; i < NUM_RULESETS; i++)
    {
        res += vec_count(rulesets + i);
    }
    return res;
}

void unload_simplification()
{
    if (!initialized) return;
    free_pattern(&deriv_before);
    free_tree(deriv_after);
    free_pattern(&malformed_deriv);
    for (size_t i = 0; i < NUM_RULESETS; i++)
    {
        free_ruleset(&rulesets[i]);
    }
    initialized = false;
}

static void apply_simplification(Node **tree, Vector *ruleset)
{
    VectorIterator it = vec_get_iterator(ruleset);
    while (apply_ruleset_by_iterator(tree, (Iterator*)&it, propositional_checker, 1) != 0)
    {
        iterator_reset((Iterator*)&it);
        tree_reduce_constant_subtrees(tree, arith_op_evaluate);
    };
    replace_negative_consts(tree);
}

/*
Summary: Applies all pre-defined rewrite rules to tree
Returns: True when transformations could be applied, False otherwise
*/
ListenerError simplify(Node **tree)
{
    ListenerError err = tree_reduce_constant_subtrees(tree, arith_op_evaluate);
    if (err != LISTENERERR_SUCCESS) return err;
    if (!initialized) return LISTENERERR_SUCCESS;

    bool tmp = set_show_errors(false);
    // Apply elimination rules
    apply_simplification(tree, rulesets + 0);

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

    Matching matching;
    Node **matched;
    while ((matched = find_matching((const Node**)tree, &deriv_before, NULL, &matching)) != NULL)
    {
        // Check if there is more than one variable in within derivative shorthand
        const char *vars[2];
        size_t var_count = list_variables(*matched, 2, vars);
        if (var_count > 1)
        {
            set_show_errors(tmp);
            return LISTENERERR_MALFORMED_DERIV_A;
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
        set_show_errors(tmp);
        return LISTENERERR_MALFORMED_DERIV_B;
    }

    // Do it twice because operators that can't be derivated could maybe be reduced
    for (size_t i = 0; i < 2; i++)
    {
        apply_simplification(tree, rulesets + 1);
        apply_simplification(tree, rulesets + 2);
        apply_simplification(tree, rulesets + 3);
        apply_simplification(tree, rulesets + 4);
        apply_simplification(tree, rulesets + 5);
        apply_simplification(tree, rulesets + 6);
    }

    // If the tree still contains deriv-operators, the user attempted to derivate 
    // a subtree for which no reduction rule exists.
    Node **unresolved_derivation = find_op((const Node**)tree, get_op(deriv_after));
    if (unresolved_derivation != NULL)
    {
        set_show_errors(tmp);
        if (get_type(get_child(*unresolved_derivation, 0)) != NTYPE_OPERATOR)
        {
            software_defect("[Simplification] Derivation failed.\n");
        }
        return LISTENERERR_IMPOSSIBLE_DERIV;
    }

    set_show_errors(tmp);
    return LISTENERERR_SUCCESS;
}
