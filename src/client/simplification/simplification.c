#include <math.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "../../engine/tree/tree_util.h"
#include "../../engine/tree/tree_to_string.h"
#include "../../engine/transformation/rewrite_rule.h"
#include "../../engine/transformation/rule_parsing.h"
#include "../../engine/parsing/parser.h"
#include "../../util/console_util.h"
#include "../../util/linked_list.h"

#include "../core/arith_context.h"
#include "../core/arith_evaluation.h"
#include "simplification.h"
#include "propositional_context.h"
#include "propositional_evaluation.h"

#define P(x) (parse_easy(g_ctx, x))
#define NUM_RULESETS 7

/*
Rulesets: 1. Elimination
          2. Derivation
          3. Normal form (e.g. flattening)
          4. Simplification
          5. Folding
          6. Prettify
          7. Ordering
*/

bool initialized = false;
Vector rulesets[NUM_RULESETS];

Pattern deriv_before;
Node *deriv_after;
Pattern malformed_deriv;

bool simplification_is_initialized()
{
    return initialized;
}

static void replace_negative_consts(Node **tree)
{
    if (get_type(*tree) == NTYPE_CONSTANT)
    {
        if (get_const_value(*tree) < 0)
        {
            Node *minus_op = malloc_operator_node(ctx_lookup_op(g_ctx, "-", OP_PLACE_PREFIX), 1, get_token_index(*tree));
            set_child(minus_op, 0, malloc_constant_node(fabs(get_const_value(*tree)), get_token_index(*tree)));
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

/*
Summary: Initializes tree simplification system
Returns: -1 if file not readable,
         -2 if ruleset file malformed,
         number of simplification rules loaded on success
*/
ssize_t init_simplification(const char *ruleset_path)
{
    FILE *ruleset_file = fopen(ruleset_path, "r");

    if (ruleset_file == NULL) return -1;

    for (size_t i = 0; i < NUM_RULESETS; i++)
    {
        rulesets[i] = get_empty_ruleset();
    }
    ssize_t num_rulesets = parse_rulesets_from_file(ruleset_file, g_propositional_ctx, NUM_RULESETS, rulesets);
    if (num_rulesets == -1)
    {
        return -2;
    }
    if (num_rulesets != NUM_RULESETS)
    {
        report_error("Too few simplification rulesets defined in %s.\n", ruleset_path);
        return -2;
    }
    fclose(ruleset_file);

    get_pattern(P("x'"), 0, NULL, &deriv_before);
    deriv_after = P("deriv(x, z)");
    parse_pattern("deriv(x, y) WHERE type(y) != VAR", g_propositional_ctx, &malformed_deriv);

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
        tree_reduce_constant_subtrees(tree, arith_op_evaluate, NULL);
    };
    replace_negative_consts(tree);
}

ListenerError apply_derivatives(Node **tree, const Node **errnode)
{
    Matching matching;
    Node **matched;
    // Transform shorthand derivative to deriv(expr, x)
    while ((matched = find_matching((const Node**)tree, &deriv_before, NULL, &matching)) != NULL)
    {
        // Check if there is more than one variable in within derivative shorthand
        const char *vars[2];
        size_t var_count = list_variables(*matched, 2, vars, NULL);
        if (var_count > 1)
        {
            if (errnode != NULL) *errnode = *matched;
            return LISTENERERR_MALFORMED_DERIV_A;
        }

        Node *replacement = tree_copy(deriv_after);
        set_token_index(replacement, get_token_index(*matched));

        if (var_count == 1)
        {
            tree_replace(get_child_addr(replacement, 1), malloc_variable_node(vars[0], 0, 0));
        }

        tree_replace(get_child_addr(replacement, 0), tree_copy(matching.mapped_nodes[0].nodes[0]));
        tree_replace(matched, replacement);
    }

    // Check for deriv(x, y) WHERE !(type(y) == VAR)
    if ((matched = find_matching((const Node**)tree, &malformed_deriv, propositional_checker, &matching)) != NULL)
    {
        if (errnode != NULL) *errnode = *matched;
        return LISTENERERR_MALFORMED_DERIV_B;
    }

    apply_simplification(tree, rulesets + 1);

    // If the tree still contains deriv-operators, the user attempted to derivate 
    // a subtree for which no reduction rule exists.
    Node **unresolved_derivation = find_op((const Node**)tree, get_op(deriv_after));
    if (unresolved_derivation != NULL)
    {
        if (errnode != NULL) *errnode = *unresolved_derivation;
        return LISTENERERR_IMPOSSIBLE_DERIV;
    }

    return LISTENERERR_SUCCESS;
}

void simplify_without_derivative(Node **tree)
{
    apply_simplification(tree, rulesets + 0);
    apply_simplification(tree, rulesets + 2);
    apply_simplification(tree, rulesets + 3);
    apply_simplification(tree, rulesets + 4);
    apply_simplification(tree, rulesets + 5);
    apply_simplification(tree, rulesets + 6);
}

/*
Summary: Applies all pre-defined rewrite rules to tree
Params
    tree:    Tree to simplify
    errnode: Node in which error occurred
Returns: True when transformations could be applied, False otherwise
*/
ListenerError simplify(Node **tree, const Node **errnode)
{
    ListenerError res = tree_reduce_constant_subtrees(tree, arith_op_evaluate, errnode);

    // It's okay if simplification module is not initialized, just return without doing much
    if (!initialized) return res;

    // Apply elimination rules
    simplify_without_derivative(tree);
    res = apply_derivatives(tree, errnode);
    if (res != LISTENERERR_SUCCESS) return res;
    simplify_without_derivative(tree);

    res = tree_reduce_constant_subtrees(tree, arith_op_evaluate, errnode);
    if (res != LISTENERERR_SUCCESS) return res;
    replace_negative_consts(tree);

    return LISTENERERR_SUCCESS;
}
