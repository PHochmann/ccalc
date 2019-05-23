#include <stdio.h>
#include <string.h>

#include "tree_to_string_test.h"

#include "../src/commands/arith_context.h"
#include "../src/engine/constants.h"
#include "../src/engine/context.h"
#include "../src/engine/node.h"
#include "../src/engine/parser.h"
#include "../src/engine/string_util.h"

#define NUM_TESTS 7

struct TreeToStringTest {
    char *string_to_tree;
    char *tree_to_string;
};

static struct TreeToStringTest tests[NUM_TESTS] = {
    { "--a", "-(-a)"},
    { "--b!!", "(-(-b)!)!"},
    { "(1+2)+3", "1+2+3" },
    { "1+(2+3)", "1+2+3" },
    { "1+2-3", "1+2-3" },
    { "1+(2-3)", "1+(2-3)" },
    
    { "-sqrt(abs(--a!!*--sum(-b+c-d+e, f^g^h-i, -sum(j, k), l+m)*--n!!))", "-sqrt(abs(((-(-a)!)!)*(-(-sum(-b+c-d+e, f^g^h-i, -sum(j, k), l+m)))*(-(-n)!)!))" }
};

int tree_to_string_test()
{
    ParsingContext *ctx = arith_get_ctx();

    for (int i = 0; i < NUM_TESTS; i++)
    {
        Node *node = NULL;
        if (parse_input(ctx, tests[i].string_to_tree, true, &node) != PERR_SUCCESS)
        {
            printf("\nError in tree inline test %d: Parser Error\n", i);
            return -1;
        }

        size_t expected_length = strlen(tests[i].tree_to_string);
        char expected_string[expected_length + 1];
        if (tree_inline(ctx, node, expected_string, expected_length + 1, false) != expected_length)
        {
            printf("\nError in tree inline test %d: Unexpected length\n", i);
            return -1;
        }

        if (strcmp(tests[i].tree_to_string, expected_string) != 0)
        {
            printf("\nError in tree inline test %d: Unexpected result\n", i);
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
