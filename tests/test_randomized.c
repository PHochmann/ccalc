#include <stdio.h>
#include <string.h>

#include "test_randomized.h"
#include "../src/arithmetics/arith_context.h"
#include "../src/tree/node.h"
#include "../src/tree/parser.h"
#include "../src/tree/tree_to_string.h"
#include "../src/string_util.h"

#define SEED                 21
#define NUM_CASES            1000
#define MAX_INNER_NODES      250
#define MAX_CONST            1000
#define MAX_DYNAMIC_CHILDREN 5

#define NUM_VARIABLE_NAMES 5
static char *variable_names[] = { "x", "y", "z", "abc", "def" };

// Restrict operators to choose from to have more interesting expressions
#define NUM_OP_INDICES 23
static size_t op_indices[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
    13, 14, 15, 16, 17, 31, 32, 40, 41, 42, 46 };

// Restrict constants to choose from to only use exactly representable numbers by CONSTANT_TYPE_FMT
#define NUM_CONSTANTS 12
static double constants[] = { 0, 1, 123, 123456, 0.5, 1.5, 123.5, 123456.5,
    0.1234567, 1.1234567, 123.2134567, 123456.1235 };

void generate_tree(size_t max_inner_nodes, Node **out)
{
    if (max_inner_nodes == 0)
    {
        // Choose constant or variable 50/50
        if (rand() % 2 == 0)
        {
            *out = malloc_constant_node(constants[rand() % NUM_CONSTANTS]);
        }
        else
        {
            // Variable with random name
            *out = malloc_variable_node(variable_names[rand() % NUM_VARIABLE_NAMES]);
        }
    }
    else
    {
        // Choose random operator
        Operator *op = &g_ctx->operators[op_indices[rand() % NUM_OP_INDICES]];
        size_t num_children;

        if (op->arity == OP_DYNAMIC_ARITY)
        {
            num_children = (size_t)(rand() % MAX_DYNAMIC_CHILDREN);
        }
        else
        {
            num_children = op->arity;
        }

        *out = malloc_operator_node(op, num_children);

        for (size_t i = 0; i < num_children; i++)
        {
            generate_tree(rand() % (1 + max_inner_nodes / num_children), get_child_addr(*out, i));
        }
    }
}

/*
Summary:
    Tests parse_input, tree_to_string and tree_equals in combination
    This test does not test them independently.
    Note that dynamic arity functions coexisting with fixed-arity function of same name cause false negatives
*/
bool randomized_test()
{
    bool error = false;
    srand(SEED);

    for (size_t i = 0; i < NUM_CASES; i++)
    {
        // Generate random tree
        Node *random_tree = NULL;
        generate_tree(MAX_INNER_NODES, &random_tree);

        // Convert random tree to string
        size_t buffer_len = tree_to_string(random_tree, NULL, 0, false) + 1;
        char stringed_tree[buffer_len];
        tree_to_string(random_tree, stringed_tree, buffer_len, false);

        // To test glue-op: replace first '*' by space
        if (rand() % 2 == 0)
        {
            char *asterisk = strstr(stringed_tree, "*");
            if (asterisk != NULL)
            {
                *asterisk = ' ';
            }
        }

        // Parse stringed random tree
        Node *parsed_tree = NULL;
        ParserError result = parse_input(g_ctx, stringed_tree, &parsed_tree);

        // Check results
        if (result != PERR_SUCCESS)
        {
            printf("[3] Parser error: %s.\n", perr_to_string(result));
            error = true;
        }
        else
        {
            if (tree_equals(random_tree, parsed_tree) != NULL)
            {
                printf("[3] Parsed tree not equal to generated tree.\n");
                error = true;
            }
        }

        if (error)
        {
            printf("Random:\n");
            print_tree_visually(random_tree);
            printf("Parsed:\n");
            print_tree_visually(parsed_tree);
            printf("String:\n%s\n", stringed_tree);
        }

        free_tree(random_tree);
        free_tree(parsed_tree);

        if (error)
        {
            return false;
        }
    }

    return true;
}

Test get_randomized_test()
{
    return (Test){
        randomized_test,
        NUM_CASES,
        "Randomized"
    };
}
