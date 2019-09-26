#include <stdio.h>
#include <string.h>

#include "tree_to_string_test.h"

#include "../src/arithmetics/arith_context.h"
#include "../src/string_util.h"

#include "../src/parsing/context.h"
#include "../src/parsing/node.h"
#include "../src/parsing/parser.h"

struct TreeToStringTest {
    char *string_to_tree;
    char *tree_to_string;
};

static const size_t NUM_TESTS = 7;
static struct TreeToStringTest tests[] = {
    { "--a", "-(-a)"},
    { "--b!!", "(-(-b)!)!"},
    { "(1+2)+3", "1+2+3" },
    { "1+(2+3)", "1+(2+3)" },
    { "1+2-3", "1+2-3" },
    { "1+(2-3)", "1+(2-3)" },
    { "-sqrt(abs(--a!!*--sum(-b+c-d+e, f^g^h-i, -sum(j, k), l+m)*--n!!))", "-sqrt(abs(((-(-a)!)!)*(-(-sum(-b+c-d+e, f^g^h-i, -sum(j, k), l+m)))*(-(-n)!)!))" }
};

int tree_to_string_test()
{
    ParsingContext *ctx = arith_init_ctx();

    for (size_t i = 0; i < NUM_TESTS; i++)
    {
        Node *node = NULL;
        if (parse_input(ctx, tests[i].string_to_tree, &node) != PERR_SUCCESS)
        {
            printf("Error in tree inline test %zu: Parser Error\n", i);
            return -1;
        }

        size_t expected_length = strlen(tests[i].tree_to_string);
        char expected_string[expected_length + 1];
        if (tree_inline(ctx, node, expected_string, expected_length + 1, false) != expected_length)
        {
            printf("Error in tree inline test %zu: Unexpected length\n", i);
            return -1;
        }

        if (strcmp(tests[i].tree_to_string, expected_string) != 0)
        {
            printf("Error in tree inline test %zu: Unexpected result\n", i);
            return -1;
        }
    }

    return 0;
}

Test get_tree_to_string_test()
{
    return (Test){
        tree_to_string_test,
        NUM_TESTS,
        "Tree to String"
    };
}
