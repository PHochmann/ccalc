#include <stdio.h>
#include <string.h>

#include "tree_to_string_test.h"
#include "../src/commands/arith_context.h"
#include "../src/engine/constants.h"
#include "../src/engine/context.h"
#include "../src/engine/node.h"
#include "../src/engine/parser.h"
#include "../src/engine/tree_to_string.h"

#define NUM_TESTS 2

typedef struct {
    char *string_to_tree;
    char *tree_to_string;
} TreeToStringTest;

static TreeToStringTest tests[NUM_TESTS] = {
    { "(1+2)+3", "1+2+3" },
    { "1+(2+3)", "1+2+3" }
};

int tree_to_string_test()
{
    ParsingContext *ctx = arith_get_ctx();

    for (int i = 0; i < NUM_TESTS; i++)
    {
        Node *node = NULL;
        if (parse_input(ctx, tests[i].string_to_tree, true, &node) != PERR_SUCCESS)
        {
            printf(F_RED "\nError in tree inline test %d: Parser Error\n" COL_RESET, i);
            return -1;
        }

        size_t expected_length = strlen(tests[i].tree_to_string);
        char expected_string[expected_length + 1];
        if (tree_inline(ctx, node, expected_string, expected_length + 1, false) != expected_length)
        {
            printf(F_RED "\nError in tree inline test %d: Unexpected length\n" COL_RESET, i);
            return -1;
        }

        if (strcmp(tests[i].tree_to_string, expected_string) != 0)
        {
            printf(F_RED "\nError in tree inline test %d: Unexpected result\n" COL_RESET, i);
            printf("%s\n", tests[i].tree_to_string);
            printf("%s\n", expected_string);
            return -1;
        }
    }

    return 0;
}