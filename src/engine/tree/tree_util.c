#include <string.h>

#include "tree_util.h"
#include "node.h"

/*
Summary: Copies tree, tree_equals(tree, copy) will return true. Source tree can be safely free'd afterwards.
*/
Node *tree_copy(const Node *tree)
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
            Node *res = malloc_variable_node(get_var_name(tree), get_id(tree));
            return res;
        }
    }
    
    return NULL;
}

/*
Summary: Checks if two trees represent exactly the same expression
Returns: First node (in-order traversal) in 'a' that is not equal to respective node in 'b', NULL otherwise
*/
bool tree_equals(const Node *a, const Node *b)
{
    if (a == NULL || b == NULL) return false;
    if (get_type(a) != get_type(b)) return false;
    
    switch (get_type(a))
    {
        case NTYPE_CONSTANT:
            if (get_const_value(a) != get_const_value(b)) return false;
            break;

        case NTYPE_VARIABLE:
            if (strcmp(get_var_name(a), get_var_name(b)) != 0 || get_id(a) != get_id(b)) return false;
            break;

        case NTYPE_OPERATOR:
            if (get_op(a) != get_op(b) || get_num_children(a) != get_num_children(b))
            {
                return false;
            }
            for (size_t i = 0; i < get_num_children(a); i++)
            {
                if (!tree_equals(get_child(a, i), get_child(b, i)))
                {
                    return false;
                }
            }
    }
    return true;
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

/*
Returns: Total number of variable nodes in tree.
    Can be used as an upper bound for the needed size of a buffer to supply to get_variable_nodes
*/
size_t count_all_variable_nodes(const Node *tree)
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
                sum += count_all_variable_nodes(get_child(tree, i));
            }
            return sum;
        }
    }
    return 0;
}

/*
Summary: Lists all pointers to variable nodes of given name
Params
    out_instances: Contains result. Function unsafe when too small. Allowed to be NULL.
Returns: Number of variable nodes found, i.e. count of out_instances
*/
size_t get_variable_nodes(const Node **tree, const char *var_name, Node ***out_instances)
{
    if (tree == NULL || var_name == NULL) return 0;

    switch (get_type(*tree))
    {
        case NTYPE_CONSTANT:
            return 0;

        case NTYPE_VARIABLE:
            if (strcmp(get_var_name(*tree), var_name) == 0)
            {
                if (out_instances != NULL)
                {
                    *out_instances = (Node**)tree; // Discards const!
                }
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
                res += get_variable_nodes((const Node**)get_child_addr(*tree, i), var_name,
                    out_instances != NULL ? out_instances + res : NULL);
            }
            return res;
        }
    }

    return 0;
}

size_t list_variables_rec(const Node *tree, size_t buffer_size, size_t num_found, const char **out_variables)
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
            if (num_found < buffer_size) out_variables[num_found] = get_var_name(tree);
            return num_found + 1;

        case NTYPE_OPERATOR:
            for (size_t i = 0; i < get_num_children(tree); i++)
            {
                num_found = list_variables_rec(get_child(tree, i), buffer_size, num_found, out_variables);
            }
            return num_found;
    }

    return 0;
}

/*
Returns: Address of first operator node that has given operator, NULL when no such node in tree
*/
Node **find_op(const Node **tree, const Operator *op)
{
    if (get_type(*tree) == NTYPE_OPERATOR)
    {
        if (get_op(*tree) == op)
        {
            return (Node**)tree;
        }
        else
        {
            for (size_t i = 0; i < get_num_children(*tree); i++)
            {
                Node **child_res = find_op((const Node**)get_child_addr(*tree, i), op);
                if (child_res != NULL) return child_res;
            }
        }
    }
    return NULL;
}

/*
Summary: Lists all variable names occurring in tree. Strings are not copied!
Returns: Length of out_variables (i.e. count of variables in tree without duplicates)
Params
    tree:          Tree to search for variables
    out_variables: Contains result. Function unsafe when too small
*/
size_t list_variables(const Node *tree, size_t buffer_size, const char **out_variables)
{
    if (tree == NULL || out_variables == NULL) return 0;
    return list_variables_rec(tree, buffer_size, 0, out_variables);
}

/*
Summary: Replaces every occurrence of a variable with a certain name by a given subtree
Returns: Number of nodes that have been replaced
Params
    tree:         Tree to search for variable occurrences
    tree_to_copy: Tree, the variables are replaced by
    var_name:     Name of variable to search for
*/
size_t replace_variable_nodes(Node **tree, const Node *tree_to_copy, const char *var_name)
{
    if (tree == NULL || *tree == NULL || tree_to_copy == NULL || var_name == NULL) return 0;

    Node **instances[count_all_variable_nodes(*tree)];
    size_t num_instances = get_variable_nodes((const Node**)tree, var_name, instances);
    for (size_t i = 0; i < num_instances; i++)
    {
        tree_replace(instances[i], tree_copy(tree_to_copy));
    }
    return num_instances;
}

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ Traversal ~ ~ ~ ~ ~ ~ ~ ~ ~ */

/*
Summary: Evaluates operator tree
Returns: True if reduction could be applied, i.e. no variable in tree and reduction-function did not return false
Params
    tree:      Tree to reduce to a constant
    reduction: Function that takes an operator, number of children, and pointer to number of children many child values
    out:       Reduction result
*/
ListenerError tree_reduce(const Node *tree, TreeListener listener, double *out)
{
    switch (get_type(tree))
    {
        case NTYPE_CONSTANT:
            *out = get_const_value(tree);
            return LISTENERERR_SUCCESS;

        case NTYPE_OPERATOR:
        {
            size_t num_args = get_num_children(tree);
            if (num_args > 0)
            {
                double args[num_args];
                for (size_t i = LISTENERERR_SUCCESS; i < num_args; i++)
                {
                    ListenerError err = tree_reduce(get_child(tree, i), listener, &args[i]);
                    if (err != LISTENERERR_SUCCESS) return err;
                }
                ListenerError err = listener(get_op(tree), num_args, args, out);
                if (err != LISTENERERR_SUCCESS) return err;
            }
            else
            {
                ListenerError err = listener(get_op(tree), 0, NULL, out);
                if (err != LISTENERERR_SUCCESS) return err;
            }
            return 0;
        }

        case NTYPE_VARIABLE:
            return LISTENERERR_VARIABLE_ENCOUNTERED;

        default:
            return -42; // This should never happen
    }
}

/*
Summary: Replaces reducible subtrees by a ConstantNode
Params:
    tree:            Tree that will be changed
    listener:        Compositional evaluation function
*/
ListenerError tree_reduce_constant_subtrees(Node **tree, TreeListener listener)
{
    if (count_all_variable_nodes(*tree) == 0)
    {
        double res;
        ListenerError err = tree_reduce(*tree, listener, &res);
        if (err != LISTENERERR_SUCCESS) return err;
        tree_replace(tree, malloc_constant_node(res));
    }
    else
    {
        if (get_type(*tree) == NTYPE_OPERATOR)
        {
            for (size_t i = 0; i < get_num_children(*tree); i++)
            {
                ListenerError err = tree_reduce_constant_subtrees(get_child_addr(*tree, i), listener);
                if (err != LISTENERERR_SUCCESS) return err;
            }
        }
    }
    return LISTENERERR_SUCCESS;
}

/*
Summary: For non-compositional evaluation of operators
*/
void tree_reduce_ops(Node **tree, const Operator *op, OpEval callback)
{
    if (get_type(*tree) == NTYPE_OPERATOR)
    {
        for (size_t i = 0; i < get_num_children(*tree); i++)
        {
            tree_reduce_ops(get_child_addr(*tree, i), op, callback);
        }

        if (get_op(*tree) == op)
        {
            tree_replace(tree, malloc_constant_node(callback(get_num_children(*tree), get_child_addr(*tree, 0))));
        }
    }
}
