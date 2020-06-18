#include "../console_util.h"
#include "../transformation/matching.h"
#include "../parsing/parser.h"
#include "../tree/tree_util.h"
#include "history.h"
#include "arith_context.h"
#include "evaluation.h"

#define ERROR_NOT_SET       "Error: This part of the history is not set yet.\n"
#define ERROR_NOT_CONSTANT  "Error: In @x, x must contain no variable.\n"
#define ERROR_OUT_OF_BOUNDS "Error: In @x, x must be between 0 and %d.\n"
#define ANS_VAR             "ans"

#define ANS_HISTORY_SIZE 10

size_t next_ans;
Node *ans[ANS_HISTORY_SIZE]; // Results of last evaluations (each evaluated to single ConstantNode)
Node *ans_pattern;

void init_history()
{
    ans_pattern = parse_conveniently(g_ctx, "@x");
    for (size_t i = 0; i < ANS_HISTORY_SIZE; i++)
    {
        ans[i] = NULL;
    }
}

void unload_history()
{
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

/*
Params
    index: 0 -> last evaluation, 1 -> second last evaluation etc.
*/
Node *get_ans(size_t index)
{
    if ((int)next_ans - 1 - (int)index < 0)
    {
        return ans[(int)next_ans - 1 - (int)index + ANS_HISTORY_SIZE];
    }
    else
    {
        return ans[next_ans - 1 - index];
    }
}

void core_update_history(ConstantType value)
{
    if (ans[next_ans] != NULL) free_tree(ans[next_ans]);
    ans[next_ans] = malloc_constant_node(value);
    next_ans = (next_ans + 1) % ANS_HISTORY_SIZE;
}

bool core_replace_history(Node **tree)
{
    // Replace @x
    Matching ans_matching;
    Node **matched_subtree;
    while ((matched_subtree = find_matching(tree, ans_pattern, &ans_matching)) != NULL)
    {
        if (count_variables(ans_matching.mapped_nodes[0].nodes[0]) > 0)
        {
            report_error(ERROR_NOT_CONSTANT);
            return false;
        }

        int index = (int)arith_evaluate(ans_matching.mapped_nodes[0].nodes[0]);

        if (index < 0 || index >= ANS_HISTORY_SIZE)
        {
            report_error(ERROR_OUT_OF_BOUNDS, ANS_HISTORY_SIZE - 1);
            return false;
        }
        if (get_ans(index) == 0)
        {
            report_error(ERROR_NOT_SET);
            return false;
        }

        tree_replace(matched_subtree, tree_copy(get_ans(index)));
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
            printf(ERROR_NOT_SET);
            return false;
        }
    }

    return true;
}
