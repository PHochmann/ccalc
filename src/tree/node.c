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
    res->id = 0;
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

char get_id(Node *node)
{
    return ((VariableNode*)node)->id;
}

void set_id(Node *node, char id)
{
    ((VariableNode*)node)->id = id;
}

ConstantType get_const_value(Node *node)
{
    return ((ConstantNode*)node)->const_value;
}
