#include <stdio.h>
#include <string.h>

#include "test_randomized.h"
#include "fuzzer.h"
#include "../src/engine/parsing/parser.h"
#include "../src/engine/tree/tree_util.h"
#include "../src/engine/tree/tree_to_string.h"
#include "../src/engine/util/string_util.h"
#include "../src/client/core/arith_context.h"

#define MAX_INNER_NODES 10
#define NUM_CASES       500

/*
Summary:
    Tests parse_input, tree_to_string and tree_equals in combination
    This test does not test them independently.
    Note that dynamic arity functions coexisting with fixed-arity function of same name cause false negatives
*/
bool randomized_test(StringBuilder *error_builder)
{
    for (size_t i = 0; i < NUM_CASES; i++)
    {
        // Generate random tree
        Node *random_tree = NULL;
        get_random_tree(MAX_INNER_NODES, &random_tree);

        // Convert random tree to string
        char *stringed_tree = tree_to_str(random_tree, false);

        // To test glue-op: replace some '*' by spaces
        while (rand() % 4 != 0)
        {
            char *asterisk = strstr(stringed_tree, "*");
            if (asterisk != NULL)
            {
                *asterisk = ' ';
            }
            else
            {
                break;
            }
        }

        // Parse stringed random tree
        Node *parsed_tree = NULL;
        ParserError result = parse_input(g_ctx, stringed_tree, &parsed_tree);

        // Check results
        if (result != PERR_SUCCESS)
        {
            ERROR("Parser error: %s.\n", perr_to_string(result));
        }
        else
        {
            if (!tree_equals(random_tree, parsed_tree))
            {
                printf("Random tree:\n");
                print_tree_visually(random_tree);
                printf("Parsed tree tree:\n");
                print_tree_visually(parsed_tree);
                ERROR("Parsed tree not equal to generated tree.\n");
            }
        }

        free_tree(random_tree);
        free_tree(parsed_tree);
        free(stringed_tree);
    }

    return true;
}

Test get_randomized_test()
{
    return (Test){
        randomized_test,
        "Randomized"
    };
}
