#include "../../engine/util/vector.h"
#include "../../engine/util/console_util.h"
#include "../../engine/transformation/matching.h"
#include "../../engine/parsing/parser.h"
#include "../../engine/tree/tree_util.h"

#include "history.h"
#include "arith_context.h"
#include "arith_evaluation.h"

#define ERROR_NOT_SET       "Error: This part of the history is not set yet.\n"
#define ERROR_NOT_CONSTANT  "Error: In @x, x must be a constant.\n"
#define ERROR_OUT_OF_BOUNDS "Error: In @x, x must be >= 0.\n"
#define ANS_VAR             "ans"

size_t next_ans;
Vector ans_vec; // Results of last evaluations (doubles)
const Operator *history_op;

void init_history()
{
    ans_vec = vec_create(sizeof(double), 5);
    history_op = ctx_lookup_op(g_ctx, "@", OP_PLACE_PREFIX);
}

void unload_history()
{
    vec_destroy(&ans_vec);
}

/*
Params
    index: 0 -> last evaluation, 1 -> second last evaluation etc.
*/
static double get_ans(size_t index)
{
    return *(double*)vec_get(&ans_vec, vec_count(&ans_vec) - 1 - index);
}

void core_add_history(double value)
{
    VEC_PUSH_ELEM(&ans_vec, double, value);
}

bool core_replace_history(Node **tree)
{
    // Replace @x
    Node **hist_subtree;
    while ((hist_subtree = find_op((const Node**)tree, history_op)) != NULL)
    {
        if (get_type(get_child(*hist_subtree, 0)) != NTYPE_CONSTANT)
        {
            report_error(ERROR_NOT_CONSTANT);
            return false;
        }

        int index = (int)get_const_value(get_child(*hist_subtree, 0));

        if (index < 0)
        {
            report_error(ERROR_OUT_OF_BOUNDS);
            return false;
        }
        if ((size_t)index >= vec_count(&ans_vec))
        {
            report_error(ERROR_NOT_SET);
            return false;
        }

        tree_replace(hist_subtree, malloc_constant_node(get_ans(index)));
    }

    // Replace normal ans
    if (get_variable_nodes((const Node**)tree, ANS_VAR, NULL) > 0)
    {
        if (vec_count(&ans_vec) > 0)
        {
            Node *n = malloc_constant_node(get_ans(0));
            replace_variable_nodes(tree, n, ANS_VAR);
            free_tree(n);
        }
        else
        {
            printf(ERROR_NOT_SET);
            return false;
        }
    }

    return true;
}
