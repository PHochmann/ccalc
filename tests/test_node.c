#include <stdlib.h>
#include <stdio.h>

#include "test_node.h"
#include "../src/tree/operator.h"
#include "../src/tree/node.h"

static const int TEST_NUMBER = 1;
static const size_t NUM_CASES = 4;

#define PRINT_ERROR_RETURN_VAL(function) {\
    printf("[%d] Unexpected return value of %s.\n", TEST_NUMBER, function);\
    error = true;\
}

// Todo: Write a test for every function in node.c

bool node_test()
{
    bool error = false;

    Operator op = op_get_function("test", OP_DYNAMIC_ARITY);

    // Manually construct tree: test(x, test(x, y), y, 42, x)
    Node *root = malloc_operator_node(&op, 5);
    Node *child = malloc_operator_node(&op, 2);
    set_child(root, 0, malloc_variable_node("x"));
    set_child(root, 1, child);
    set_child(root, 2, malloc_variable_node("y"));
    set_child(root, 3, malloc_constant_node(42));
    set_child(root, 4, malloc_variable_node("x"));
    set_child(child, 0, malloc_variable_node("x"));
    set_child(child, 1, malloc_variable_node("y"));

    // Case 1
    if (count_variables(root) != 5)
    {
        PRINT_ERROR_RETURN_VAL("count_variables");
    }

    // Case 2
    if (count_variables_distinct(root) != 2)
    {
        PRINT_ERROR_RETURN_VAL("count_variables_distinct");
    }

    // Case 3
    Node **vars_x[3];
    if (get_variable_nodes(&root, "x", vars_x) != 3)
    {
        PRINT_ERROR_RETURN_VAL("get_variable_nodes");
    }

    if (*vars_x[0] != get_child(root, 0)
        || *vars_x[1] != get_child(get_child(root, 1), 0)
        || *vars_x[2] != get_child(root, 4))
    {
        printf("[%d] Unexpected out_instances of get_variable_nodes.\n", TEST_NUMBER);
        return false;
    }

    // Case 4
    Node *replacement = tree_copy(child);
    if (replace_variable_nodes(&root, child, "y") != 2)
    {
        PRINT_ERROR_RETURN_VAL("replace_variable_nodes");
    }

    free_tree(replacement);
    free_tree(root);

    return !error;
}

Test get_node_test()
{
    return (Test){
        node_test,
        NUM_CASES,
        "Node"
    };
}
