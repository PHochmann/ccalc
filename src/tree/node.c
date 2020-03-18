#include <string.h>
#include "node.h"

/*
The following functions are used for polymorphism of different Node types
*/

Node *malloc_variable_node(char *var_name)
{
    VariableNode *res = malloc(sizeof(VariableNode) + (strlen(var_name) + 1) * sizeof(char));
    if (res == NULL) return NULL;
    res->base.type = NTYPE_VARIABLE;
    strcpy(res->var_name, var_name);
    return (Node*)res;
}

Node *malloc_constant_node(ConstantType value)
{
    ConstantNode *res = malloc(sizeof(ConstantNode));
    if (res == NULL) return NULL;
    res->base.type = NTYPE_CONSTANT;
    res->const_value = value;
    return (Node*)res;
}

Node *malloc_operator_node(Operator *op, size_t num_children)
{
    if (num_children > OP_MAX_ARITY)
    {
        return NULL;
    }

    OperatorNode *res = malloc(sizeof(OperatorNode) + num_children * sizeof(Node*));
    if (res == NULL) return NULL;
    
    for (size_t i = 0; i < num_children; i++)
    {
        res->children[i] = NULL;
    }

    res->base.type = NTYPE_OPERATOR;
    res->op = op;
    res->num_children = num_children;
    return (Node*)res;
}

void free_tree(Node *tree)
{
    if (tree == NULL) return;
    if (get_type(tree) == NTYPE_OPERATOR)
    {
        for (size_t i = 0; i < get_num_children(tree); i++)
        {
            free_tree(get_child(tree, i));
        }
    }
    free(tree);
}

NodeType get_type(Node *node)
{
    return node->type;
}

Operator *get_op(Node *node)
{
    return ((OperatorNode*)node)->op;
}

size_t get_num_children(Node *node)
{
    return ((OperatorNode*)node)->num_children;
}

Node *get_child(Node *node, size_t index)
{
    return ((OperatorNode*)node)->children[index];
}

Node **get_child_addr(Node *node, size_t index)
{
    return &((OperatorNode*)node)->children[index];
}

void set_child(Node *node, size_t index, Node *child)
{
    ((OperatorNode*)node)->children[index] = child;
}

char *get_var_name(Node *node)
{
    return ((VariableNode*)node)->var_name;
}

ConstantType get_const_value(Node *node)
{
    return ((ConstantNode*)node)->const_value;
}

/*
Summary: Copies tree, tree_equals will be true of copy. Source tree can be safely free'd afterwards.
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
            return malloc_variable_node(get_var_name(tree));
    }
    
    return NULL;
}

/*
Summary: Checks if two trees represent exactly the same expression
Returns: First node (in-order traversal) in 'a' that is not equal to respective node in 'b', NULL otherwise
*/
Node *tree_equals(Node *a, Node *b)
{
    if (a == NULL || b == NULL) return NULL;
    if (get_type(a) != get_type(b)) return a;
    
    switch (get_type(a))
    {
        case NTYPE_CONSTANT:
            if (get_const_value(a) != get_const_value(b)) return a;
            break;

        case NTYPE_VARIABLE:
            if (strcmp(get_var_name(a), get_var_name(b)) != 0) return a;
            break;

        case NTYPE_OPERATOR:
            if (get_op(a) != get_op(b) || get_num_children(a) != get_num_children(b))
            {
                return a;
            }
            for (size_t i = 0; i < get_num_children(a); i++)
            {
                Node *recursive_res = tree_equals(get_child(a, i), get_child(b, i));
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

// What follows are helper functions commonly used to search trees

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
bool reduce(Node *tree, Evaluation reduction, ConstantType *out)
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
                if (!reduce(get_child(tree, i), reduction, &args[i]))
                {
                    return false;
                }
            }
            if (!reduction(get_op(tree), num_args, args, out))
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

ConstantType convenient_reduce(Node *tree, Evaluation reduction)
{
    ConstantType res = 0;
    reduce(tree, reduction, &res);
    return res;
}