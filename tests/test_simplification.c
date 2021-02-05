#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../src/engine/tree/operator.h"
#include "../src/engine/tree/tree_util.h"
#include "../src/engine/tree/tree_to_string.h"
#include "../src/engine/parsing/parser.h"
#include "../src/client/core/arith_context.h"
#include "../src/client/core/arith_evaluation.h"
#include "../src/client/simplification/simplification.h"
#include "fuzzer.h"
#include "test_simplification.h"

/*#define NUM_FUZZER_CASES 300
#define MAX_INNER_NODES  100
#define EPS 0.001*/

static const size_t NUM_CASES = 19;
char *cases[] = {
    "x-x",                 "0",
    "x+x",                 "2x",
    "x+x+x+x+x",           "5x",
    "x-x-x-x-x",           "-3x",
    "10x-x-x-x-10x",       "-3x",
    "2*(sqrt(2)*x)^2",     "4x^2", // Precision problems
    "(-x)^2 - x^2",        "0",
    "(-x)^(1+1)",          "x^2",
    "(2x)/(4x)",           "0.5",
    "5x-6x",               "-x",
    "100x%",               "x",
    "(x^2+x^3)x^4",        "x^7+x^6",
    "sqrt(x)/sqrt(x y)",   "1/sqrt(y)",
    "avg(a,b)",            "0.5a+0.5b",
    
    // Derivative
    "4'",                  "0",
    "x'",                  "1",
    "(10x^10)'''''''''''", "0",
    "deriv(3*x*y, y)",     "3x",
    "deriv((3x)'*y, y)",   "3"
};

bool simplification_test(StringBuilder *error_builder)
{
    for (size_t i = 0; i < NUM_CASES; i++)
    {
        Node *left = NULL;
        if (parse_input(g_ctx, cases[2 * i], &left, NULL) != PERR_SUCCESS)
        {
            ERROR("Syntax error in left side of test case %zu.\n", i);
        }
        Node *right = NULL;
        if (parse_input(g_ctx, cases[2 * i + 1], &right, NULL) != PERR_SUCCESS)
        {
            ERROR("Syntax error in right side of test case %zu.\n", i);
        }
        if (simplify(&left) != LISTENERERR_SUCCESS)
        {
            ERROR("core_simplify returned false in test case %zu.\n", i);
        }

        if (!tree_equals(left, right))
        {
            // Additional test: Double precision may cause problems, string tree and see if equals
            char *wrong_result = tree_to_str(left, true);
            char *right_result = tree_to_str(right, true);

            if (strcmp(wrong_result, right_result) != 0)
            {
                ERROR("%s simplified to %s, should be %s.\n", cases[2 * i], wrong_result, right_result);
            }
            else
            {
                // Carry on, test case is passed
                free(wrong_result);
                free(right_result);
            }
        }
        
        free_tree(left);
        free_tree(right);
    }

    // Fuzzer test to detect illegal simplification rules
    /*for (size_t i = 0; i < NUM_FUZZER_CASES; i++)
    {
        Node *tree = NULL;
        get_random_tree(MAX_INNER_NODES, &tree);
        Node *cc = tree_copy(tree);
        const char *variables[40];
        Node *copy = tree_copy(tree);
        size_t num_variables = list_variables(copy, 40, variables);
        simplify(&tree);
        for (size_t j = 0; j < num_variables; j++)
        {
            Node *replacement = malloc_constant_node(((double)(rand() % 10000) / 100));
            replace_variable_nodes(&copy, replacement, variables[j]);
            replace_variable_nodes(&tree, replacement, variables[j]);
            free_tree(replacement);
        }
        double before_result = arith_evaluate(copy);
        double after_result = arith_evaluate(tree);

        if (before_result - after_result > EPS)
        {
            printf("Num variables: %zu\n", num_variables);
            print_tree(cc, true);
            printf("\n");
            print_tree(tree, true);
            printf("\n");
            print_tree(copy, true);
            printf("\n");
            ERROR("Fuzzing test failed\n");
        }

        free_tree(tree);
        free_tree(copy);
    }*/

    return true;
}

Test get_simplification_test()
{
    return (Test){
        simplification_test,
        "Simplification"
    };
}
