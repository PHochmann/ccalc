#include <stdlib.h>
#include <stdio.h>

#include "test_simplification.h"
#include "../src/tree/operator.h"
#include "../src/tree/tree_util.h"
#include "../src/tree/tree_to_string.h"
#include "../src/core/arith_context.h"
#include "../src/core/simplification.h"

#define ERROR(fmt, ...) {\
    strbuilder_append(error_builder, fmt, __VA_ARGS__);\
    return false;\
}

static const size_t NUM_CASES = 7;
char *cases[] = {
    "x-x",             "0",
    "x+x",             "2x",
    "2*(sqrt(2)*x)^2", "4x^2",
    "(-x)^2 - x^2",    "0",
    "(-x)^(1+1)",      "x^2",
    "x'",              "1",
    "(2x)/(4x)",       "0.5"
};

bool simplification_test(Vector *error_builder)
{
    for (size_t i = 0; i < NUM_CASES; i++)
    {
        Node *left = NULL;
        if (parse_input(g_ctx, cases[2 * i], &left) != PERR_SUCCESS)
        {
            ERROR("Syntax error in left side of test case %zu.\n", i);
        }
        Node *right = NULL;
        if (parse_input(g_ctx, cases[2 * i + 1], &right) != PERR_SUCCESS)
        {
            ERROR("Syntax error in right side of test case %zu.\n", i);
        }
        if (!core_simplify(&left, true))
        {
            ERROR("core_simplify returned false in test case %zu.\n", i);
        }
        if (tree_compare(left, right) != NULL)
        {
            char *wrong_result = tree_to_str(left, false);
            strbuilder_append(error_builder, "%s simplified to %s, should be %s.\n", cases[2 * i], wrong_result, cases[2 * i + 1]);
            free(wrong_result);
            free_tree(left);
            free_tree(right);
            return false;
        }
        free_tree(left);
        free_tree(right);
    }
    return true;
}

Test get_simplification_test()
{
    return (Test){
        simplification_test,
        NUM_CASES,
        "Simplification"
    };
}
