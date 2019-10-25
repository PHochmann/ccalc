#include <stdio.h>

#include "../src/parsing/node.h"
#include "../src/parsing/parser.h"
#include "../src/util/string_util.h"
#include "../src/arithmetics/arith_context.h"

#include "massive_test.h"

#define SEED                 24
#define NUM_CASES            500
#define MAX_INNER_NODES      250
#define MAX_CONST            1000
#define MAX_DYNAMIC_CHILDREN 4

#define NUM_VARIABLE_NAMES   5
static char *variable_names[] = { "x", "y", "z", "joy", "division" };

void generate_tree(size_t max_inner_nodes, Node **out)
{
    if (max_inner_nodes == 0)
    {
        // Choose constant or variable 50/50
        if (rand() % 2 == 0)
        {
            // Constant with random value
            // Don't include negative constants because parse-tree wouldn't be unique due to prefix '-' operator
            double val = (int)(((float)rand() / (float)RAND_MAX) * MAX_CONST);
            // Don't include wild fractions due to truncation
            if (rand() % 2 == 0) val += 0.5;

            *out = malloc_constant_node(val);
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
        // Don't pick operators[0] because it's $
        Operator *op = &g_ctx->operators[1 + rand() % (ARITH_NUM_OPS - 1)];
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
            generate_tree(rand() % (max_inner_nodes / num_children + 1), get_child_addr(*out, i));
        }
    }
}

/*
Summary:
    Tests parse_input, tree_to_string and tree_equals in combination
    This test does not test them independently.
    Note that dynamic arity functions coexisting with fixed-arity function of same name cause false negatives
*/
bool massive_test()
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

        // Parse stringed random tree
        Node *parsed_tree = NULL;
        ParserError result = parse_input(g_ctx, stringed_tree, &parsed_tree);

        // Check results
        if (result != PERR_SUCCESS)
        {
            printf("[2] Could not parse tree %zu successfully: %s.\n", i, perr_to_string(result));
            error = true;
        }
        else
        {
            if (tree_equals(random_tree, parsed_tree) != NULL)
            {
                printf("[2] Parsed tree %zu not equal to generated tree.\n", i);
                error = true;
            }
        }

        if (error)
        {
            printf("~ ~ DUMP ~ ~\n"
                   "%s\n", stringed_tree);
            print_tree_visually(random_tree);
            print_tree_visually(parsed_tree);
            printf("~ ~ ~ ~ ~ ~ ~\n");
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

Test get_massive_test()
{
    return (Test){
        massive_test,
        NUM_CASES,
        "Randomized"
    };
}
