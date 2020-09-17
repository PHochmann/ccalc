#include <stdlib.h>
#include <stdio.h>

#include "test_tree_util.h"
#include "../src/tree/operator.h"
#include "../src/tree/tree_util.h"
#include "../src/tree/tree_to_string.h"

bool tree_util_test(StringBuilder *error_builder)
{
    Operator op = op_get_function("test", OP_DYNAMIC_ARITY);

    // Manually construct tree: test(x, test(x, y), y, 42, x)
    Node *root = malloc_operator_node(&op, 5);
    Node *child = malloc_operator_node(&op, 2);
    set_child(root, 0, malloc_variable_node("x", 0));
    set_child(root, 1, child);
    set_child(root, 2, malloc_variable_node("y", 0));
    set_child(root, 3, malloc_constant_node(42));
    set_child(root, 4, malloc_variable_node("x", 0));
    set_child(child, 0, malloc_variable_node("x", 0));
    set_child(child, 1, malloc_variable_node("y", 0));

    // Case 1
    if (count_variables(root) != 5)
    {
        free_tree(root);
        ERROR_RETURN_VAL("count_variables");
    }

    // Case 2
    char *nodes[2];
    if (list_variables(root, 2, nodes) != 2)
    {
        free_tree(root);
        ERROR_RETURN_VAL("list_variables without buffer");
    }

    // Case 3
    Node **vars_x[3];
    if (get_variable_nodes(&root, "x", vars_x) != 3)
    {
        free_tree(root);
        ERROR_RETURN_VAL("get_variable_nodes");
    }

    if (*vars_x[0] != get_child(root, 0)
        || *vars_x[1] != get_child(get_child(root, 1), 0)
        || *vars_x[2] != get_child(root, 4))
    {
        free_tree(root);
        ERROR("Unexpected out_instances of get_variable_nodes.\n");
    }

    // Case 4
    if (tree_compare(root, child) == NULL)
    {
        free_tree(root);
        ERROR_RETURN_VAL("tree_compare");
    }

    // Case 5
    // Check correct replacement by building new tree by hand
    Node *root_copy = tree_copy(root);
    Node *child_copy = tree_copy(child);
    free_tree(get_child(root_copy, 2));
    set_child(root_copy, 2, tree_copy(child));
    free_tree(get_child(get_child(root_copy, 1), 1));
    set_child(get_child(root_copy, 1), 1, tree_copy(child));

    // Replace
    Node *replacement = tree_copy(child);
    if (replace_variable_nodes(&root, child_copy, "y") != 2)
    {
        free_tree(root);
        free_tree(root_copy);
        free_tree(child_copy);
        free_tree(replacement);
        ERROR_RETURN_VAL("replace_variable_nodes");
    }
    
    // Check equality
    if (tree_compare(root_copy, root) != NULL)
    {
        ERROR("Unexpected replacement by replace_variable_nodes (or tree_copy broken).\n");
    }

    free_tree(root);
    free_tree(root_copy);
    free_tree(child_copy);
    free_tree(replacement);
    return true;
}

Test get_tree_util_test()
{
    return (Test){
        tree_util_test,
        "Tree util"
    };
}
