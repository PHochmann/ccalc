#include <stdlib.h>
#include "../../engine/tree/node.h"
#include "../../client/core/arith_context.h"
#include "fuzzer.h"

#define SEED                  21
#define MAX_DYNAMIC_CHILDREN   5

#define NUM_VARIABLE_NAMES 5
static char *variable_names[] = { "x", "y", "z", "abc", "def" };

// Restrict operators to choose from to have more interesting expressions
#define NUM_OP_INDICES 18
static size_t op_indices[] = { 4, 5, 6, 7, 10, 11, 12, 14,
    16, 17, 18, 19, 20, 34, 35, 43, 44, 45 };

// Restrict constants to choose from to only use exactly representable numbers by CONSTANT_TYPE_FMT
#define NUM_CONSTANTS 12
static double constants[] = { 0, 1, 2, 3, 0.5, 1.5, 2.5, 3.5,
    0.1234567, 1.1234567, 2.2134567, 3.1235 };

void get_random_tree(size_t max_inner_nodes, Node **out)
{
    if (max_inner_nodes == 0)
    {
        // Choose constant or variable 50/50
        if (rand() % 2 == 0)
        {
            *out = malloc_constant_node(constants[rand() % NUM_CONSTANTS], 0);
        }
        else
        {
            // Variable with random name
            *out = malloc_variable_node(variable_names[rand() % NUM_VARIABLE_NAMES], 0, 0);
        }
    }
    else
    {
        // Choose random operator
        Operator *op = list_get_at(&g_ctx->op_list, op_indices[rand() % NUM_OP_INDICES]);
        size_t num_children;

        if (op->arity == OP_DYNAMIC_ARITY)
        {
            num_children = (size_t)(rand() % MAX_DYNAMIC_CHILDREN);
        }
        else
        {
            num_children = op->arity;
        }

        *out = malloc_operator_node(op, num_children, 0);

        for (size_t i = 0; i < num_children; i++)
        {
            get_random_tree(rand() % (1 + max_inner_nodes / num_children), get_child_addr(*out, i));
        }
    }
}
