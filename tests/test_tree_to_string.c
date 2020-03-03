#include <stdio.h>
#include <string.h>

#include "test_tree_to_string.h"
#include "../src/tree/tree_to_string.h"
#include "../src/tree/context.h"
#include "../src/tree/node.h"
#include "../src/tree/parser.h"
#include "../src/core/arith_context.h"

struct TreeToStringTest {
    char *input;
    char *expected_result;
};

static const size_t NUM_CASES = 12;
static struct TreeToStringTest tests[] = {
    { "--a",      "-(-a)"},
    { "--b!!",    "(-(-b)!)!"},
    { "(1+2)+3",  "1+2+3" },
    { "1+(2+3)", "1+(2+3)" },
    { "1+2-3",    "1+2-3" },
    { "1+(2-3)",  "1+(2-3)" },
    { "pi",       "pi" },
    { "log(1,2)", "log(1,2)"},
    { "sum",      "sum()" },
    { "sin--1" ,  "sin(-(-1))" },
    { "sum--1",   "sum()-(-1)" },
    { "-sqrt(abs(--a!!*--sum(-b+c-d+e, f^(g^h)-i, -sum(j, k), l+m)*--n!!))",
        "-sqrt(abs(((-(-a)!)!)*(-(-sum(-b+c-d+e,f^g^h-i,-sum(j,k),l+m)))*(-(-n)!)!))" },
};

char *tree_to_string_test()
{
    core_init_ctx();
    char *res = NULL;
    Node *node = NULL;

    for (size_t i = 0; i < NUM_CASES; i++)
    {
        size_t expected_length = strlen(tests[i].expected_result);
        char result[expected_length + 1];

        if (parse_input(g_ctx, tests[i].input, &node) != PERR_SUCCESS)
        {
            return create_error("Parser Error in '%s'\n", tests[i].input);
        }

        // Check if tree_to_string returns correct length with and without buffer
        if (tree_to_string(node, NULL, 0, false) != expected_length
            || tree_to_string(node, result, expected_length + 1, false) != expected_length)
        {
            res = create_error("Unexpected length in '%s'\n", tests[i].input);
            goto error;
        }

        if (strcmp(tests[i].expected_result, result) != 0)
        {
            res = create_error("Unexpected result in '%s'\n", tests[i].input);
            goto error;
        }

        free_tree(node);
    }

    return NULL;

    error:
    free_tree(node);
    return res;
}

Test get_tree_to_string_test()
{
    return (Test){
        tree_to_string_test,
        NUM_CASES,
        "Tree to string"
    };
}
