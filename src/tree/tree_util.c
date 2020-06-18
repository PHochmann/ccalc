#include <string.h>

#include "tree_util.h"
#include "node.h"

/*
Summary: Copies tree, tree_compare(tree, copy) will return NULL. Source tree can be safely free'd afterwards.
*/
Node *tree_copy(Node *tree)
{
    if (tree == NULL) return NULL;

    switch (get_type(tree))
    {
        case NTYPE_OPERATOR:
        {
            Node *res = malloc_operator_node(get_op(tree), get_num_children(tree));
            for (size_t i = 0; i < get_num_children(tree); i++)
            {
                set_child(res, i, tree_copy(get_child(tree, i)));
            }
            return res;
        }
        
        case NTYPE_CONSTANT:
            return malloc_constant_node(get_const_value(tree));
            
        case NTYPE_VARIABLE:
        {
            Node *res = malloc_variable_node(get_var_name(tree));
            set_id(res, get_id(tree));
            return res;
        }
    }
    
    return NULL;
}

/*
Summary: Checks if two trees represent exactly the same expression
Returns: First node (in-order traversal) in 'a' that is not equal to respective node in 'b', NULL otherwise
*/
Node *tree_compare(Node *a, Node *b)
{
    if (a == NULL || b == NULL) return NULL;
    if (get_type(a) != get_type(b)) return a;
    
    switch (get_type(a))
    {
        case NTYPE_CONSTANT:
            if (get_const_value(a) != get_const_value(b)) return a;
            break;

        case NTYPE_VARIABLE:
            if (strcmp(get_var_name(a), get_var_name(b)) != 0 || get_id(a) != get_id(b)) return a;
            break;

        case NTYPE_OPERATOR:
            if (get_op(a) != get_op(b) || get_num_children(a) != get_num_children(b))
            {
                return a;
            }
            for (size_t i = 0; i < get_num_children(a); i++)
            {
                Node *recursive_res = tree_compare(get_child(a, i), get_child(b, i));
                if (recursive_res != NULL) return recursive_res;
            }
    }
    return NULL;
}

/*
Summary: Frees *tree_to_replace and assigns tree_to_insert to tree_to_replace
*/
void tree_replace(Node **tree_to_replace, Node *tree_to_insert)
{
    free_tree(*tree_to_replace);
    *tree_to_replace = tree_to_insert;
}

/*
Returns: Total number of variable nodes in tree.
    Can be used as an upper bound for the needed size of a buffer to supply to get_variable_nodes
*/
size_t count_variables(Node *tree)
{
    if (tree == NULL) return 0;

    switch (get_type(tree))
    {
        case NTYPE_CONSTANT:
            return 0;

        case NTYPE_VARIABLE:
            return 1;

        case NTYPE_OPERATOR:
        {
            size_t sum = 0;
            for (size_t i = 0; i < get_num_children(tree); i++)
            {
                sum += count_variables(get_child(tree, i));
            }
            return sum;
        }
    }
    return 0;
}

/*
Summary: Variant of list_variables that discards 'out_variables'
Returns: Number of different variables present in tree
*/
size_t count_variables_distinct(Node *tree)
{
    char *vars[count_variables(tree)];
    return list_variables(tree, vars);
}

/*
Summary: Lists all pointers to variable nodes of given name
Params
    out_instances: Contains result. Function unsafe when too small.
Returns: Number of variable nodes found, i.e. count of out_instances
*/
size_t get_variable_nodes(Node **tree, char *var_name, Node ***out_instances)
{
    if (tree == NULL || var_name == NULL || out_instances == NULL) return 0;

    switch (get_type(*tree))
    {
        case NTYPE_CONSTANT:
            return 0;

        case NTYPE_VARIABLE:
            if (strcmp(get_var_name(*tree), var_name) == 0)
            {
                *out_instances = tree;
                return 1;
            }
            else
            {
                return 0;
            }

        case NTYPE_OPERATOR:
        {
            size_t res = 0;
            for (size_t i = 0; i < get_num_children(*tree); i++)
            {
                res += get_variable_nodes(get_child_addr(*tree, i), var_name, out_instances + res);
            }
            return res;
        }
    }

    return 0;
}

/*
Summary: Variant of get_variable_nodes that discards 'out_instances'
*/
size_t count_variable_nodes(Node *tree, char *var_name)
{
    // out-discard pattern
    Node **instances[count_variables(tree)];
    return get_variable_nodes(&tree, var_name, instances);
}

size_t list_variables_rec(Node *tree, size_t num_found, char **out_variables)
{
    switch (get_type(tree))
    {
        case NTYPE_CONSTANT:
            return num_found;
        
        case NTYPE_VARIABLE:
            // Check if we already found variable
            for (size_t i = 0; i < num_found; i++)
            {
                if (strcmp(get_var_name(tree), out_variables[i]) == 0)
                {
                    return num_found;
                }
            }
            out_variables[num_found] = get_var_name(tree);
            return num_found + 1;

        case NTYPE_OPERATOR:
            for (size_t i = 0; i < get_num_children(tree); i++)
            {
                num_found = list_variables_rec(get_child(tree, i), num_found, out_variables);
            }
            return num_found;
    }

    return 0;
}

/*
Summary: Lists all variable names occurring in tree. Strings are not copied!
Params
    tree:          Tree to search for variables
    out_variables: Contains result. Function unsafe when too small.
Returns: Length of out_variables (i.e. count of variables in tree without duplicates)
*/
size_t list_variables(Node *tree, char **out_variables)
{
    if (tree == NULL || out_variables == NULL) return 0;
    return list_variables_rec(tree, 0, out_variables);
}

/*
Summary: Replaces every occurrence of a variable with a certain name by a given subtree
Params
    tree:         Tree to search for variable occurrences
    tree_to_copy: Tree, the variables are replaced by
    var_name:     Name of variable to search for
Returns: Number of nodes that have been replaced
*/
size_t replace_variable_nodes(Node **tree, Node *tree_to_copy, char *var_name)
{
    if (tree == NULL || *tree == NULL || tree_to_copy == NULL || var_name == NULL) return 0;

    Node **instances[count_variables(*tree)];
    size_t num_instances = get_variable_nodes(tree, var_name, instances);
    for (size_t i = 0; i < num_instances; i++)
    {
        tree_replace(instances[i], tree_copy(tree_to_copy));
    }
    return num_instances;
}

/*
Summary: Evaluates operator tree
Returns: True if reduction could be applied, i.e. no variable in tree and reduction-function did not return false
Params
    tree:      Tree to reduce to a constant
    reduction: Function that takes an operator, number of children, and pointer to number of children many child values
    out:       Reduction result
*/
bool reduce(Node *tree, Evaluation eval, ConstantType *out)
{
    switch (get_type(tree))
    {
        case NTYPE_CONSTANT:
            *out = get_const_value(tree);
            return true;

        case NTYPE_OPERATOR:
        {
            size_t num_args = get_num_children(tree);
            ConstantType args[num_args];
            for (size_t i = 0; i < num_args; i++)
            {
                if (!reduce(get_child(tree, i), eval, &args[i]))
                {
                    return false;
                }
            }
            if (!eval(get_op(tree), num_args, args, out))
            {
                return false;
            }
            return true;
        }

        case NTYPE_VARIABLE:
            return false;
    }
    return false;
}

ConstantType convenient_reduce(Node *tree, Evaluation eval)
{
    ConstantType res = 0;
    reduce(tree, eval, &res);
    return res;
}

void replace_constant_subtrees(Node **tree, Evaluation eval)
{
    if (count_variables(*tree) == 0)
    {
        ConstantType res = convenient_reduce(*tree, eval);
        tree_replace(tree, malloc_constant_node(res));
    }
    else
    {
        if (get_type(*tree) == NTYPE_OPERATOR)
        {
            for (size_t i = 0; i < get_num_children(*tree); i++)
            {
                replace_constant_subtrees(get_child_addr(*tree, i), eval);
            }
        }
    }
}

// Unschöne Methoden folgen
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Todo: Refactor or replace this:

/*
Nodes from list are copied, the others are not
*/
void tree_replace_by_list(Node **parent, size_t child_to_replace, NodeList list)
{
    if (list.size != 1) // Parent needs to be replaced (but not via tree_replace because most children are preserved)
    {
        free_tree(get_child(*parent, child_to_replace));
        Node *new_parent = malloc_operator_node(get_op(*parent), get_num_children(*parent) - 1 + list.size);
        for (size_t i = 0; i < child_to_replace; i++)
        {
            set_child(new_parent, i, get_child(*parent, i));
        }
        for (size_t i = 0; i < list.size; i++)
        {
            set_child(new_parent, child_to_replace + i, tree_copy(list.nodes[i]));
        }
        for (size_t i = 0; i < get_num_children(*parent) - child_to_replace - 1; i++)
        {
            set_child(new_parent,
                child_to_replace + list.size + i,
                get_child(*parent, child_to_replace + i + 1));
        }
        free(*parent);
        *parent = new_parent;
    }
    else
    {
        tree_replace(get_child_addr(*parent, child_to_replace), tree_copy(list.nodes[0]));
    }
}

size_t replace_variable_nodes_by_list(Node **tree, NodeList list_of_nodes_to_copy, char *var_name, char id)
{
    if (get_type(*tree) != NTYPE_OPERATOR)
    {
        if (list_of_nodes_to_copy.size != 1)
        {
            //Software defect: Can't substitute a tree that consists of a single node with a list with more than one node
            return 0;
        }

        if (get_type(*tree) == NTYPE_CONSTANT) return 0;
        if (get_type(*tree) == NTYPE_VARIABLE)
        {
            if (strcmp(get_var_name(*tree), var_name) == 0 && get_id(*tree) == id)
            {
                tree_replace(tree, tree_copy(list_of_nodes_to_copy.nodes[0]));
                return 1;
            }
        }
        return 0;
    }
    else
    {
        size_t replacement_counter = 0;
        for (size_t i = 0; i < get_num_children(*tree); i++)
        {
            if (get_type(get_child(*tree, i)) == NTYPE_VARIABLE)
            {
                if (strcmp(get_var_name(get_child(*tree, i)), var_name) == 0 && get_id(get_child(*tree, i)) == id)
                {
                    tree_replace_by_list(tree, i, list_of_nodes_to_copy);
                    replacement_counter++;
                }
            }
            else
            {
                replacement_counter +=
                    replace_variable_nodes_by_list(get_child_addr(*tree, i), list_of_nodes_to_copy, var_name, id);
            }
        }
        return replacement_counter;
    }
}
