#include <stdio.h>
#include <string.h>

#include "../src/engine/parsing/context.h"
#include "../src/engine/parsing/parser.h"
#include "../src/engine/tree/node.h"
#include "../src/engine/tree/tree_to_string.h"
#include "../src/client/core/arith_context.h"
#include "test_tree_to_string.h"

struct TreeToStringTest {
    char *input;
    char *expected_result;
};

static const size_t NUM_CASES = 13;
static struct TreeToStringTest tests[] = {
    { "--a",      "-(-a)"},
    { "--b!!",    "(-(-b)!)!"},
    { "(1+2)+3",  "1+2+3" },
    { "1+(2+3)", "1+(2+3)" },
    { "1+2-3",    "1+2-3" },
    { "1+(2-3)",  "1+(2-3)" },
    { "pi",       "pi" },
    { "log(1,2)", "log(1,2)"},
    { "sum()",      "sum()" },
    { "sum(--1)",   "sum(-(-1))" },
    { "-sqrt(abs(--a!!*--sum(-b+c-d+e, f^(g^h)-i, -sum(j, k), l+m)*--n!!))",
        "-sqrt(abs(((-(-a)!)!)*(-(-sum(-b+c-d+e,f^g^h-i,-sum(j,k),l+m)))*(-(-n)!)!))" },
    // Test glue op truncation
    { "10*x", "10x" },
    { "10*x^8", "10x^8" }
};

bool tree_to_string_test(StringBuilder *error_builder)
{
    for (size_t i = 0; i < NUM_CASES; i++)
    {
        Node *node = parse_easy(g_ctx, tests[i].input);
        if (node == NULL)
        {
            ERROR("Parser Error in '%s'\n", tests[i].input);
        }

        char *result = tree_to_str_fancy(node, g_ctx, false);

        if (strcmp(tests[i].expected_result, result) != 0)
        {
            ERROR("Unexpected result in '%s'. Should be '%s', is '%s'\n", tests[i].input, tests[i].expected_result, result);
        }

        free(result);
        free_tree(node);
    }

    return true;
}

Test get_tree_to_string_test()
{
    return (Test){
        tree_to_string_test,
        "Tree to string"
    };
}
