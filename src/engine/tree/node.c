#include <string.h>
#include "../util/alloc_wrappers.h"
#include "../util/console_util.h"
#include "node.h"

struct Node {
    NodeType type;
    size_t token_index;
};

typedef struct {
    Node base;
    size_t id; // For easier lookup
    char var_name[];
} VariableNode;

typedef struct {
    Node base;
    double const_value;
} ConstantNode;

typedef struct {
    Node base;
    const Operator *op;  // Points to operator in context
    size_t num_children; // Size of children buffer
    Node *children[];
} OperatorNode;

/*
The following functions are used for polymorphism of different Node types
*/

Node *malloc_variable_node(const char *var_name, size_t id, size_t tok_index)
{
    VariableNode *res = malloc_wrapper(sizeof(VariableNode) + (strlen(var_name) + 1) * sizeof(char));
    res->base.type = NTYPE_VARIABLE;
    res->base.token_index = tok_index;
    res->id = id;
    strcpy(res->var_name, var_name);
    return (Node*)res;
}

Node *malloc_constant_node(double value, size_t tok_index)
{
    ConstantNode *res = malloc_wrapper(sizeof(ConstantNode));
    res->base.type = NTYPE_CONSTANT;
    res->base.token_index = tok_index;
    res->const_value = value;
    return (Node*)res;
}

Node *malloc_operator_node(const Operator *op, size_t num_children, size_t tok_index)
{
    if (num_children > MAX_CHILDREN) software_defect("malloc operator node: too many children\n");

    OperatorNode *res = malloc_wrapper(sizeof(OperatorNode) + num_children * sizeof(Node*));
    for (size_t i = 0; i < num_children; i++) res->children[i] = NULL;
    res->base.type = NTYPE_OPERATOR;
    res->base.token_index = tok_index;
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

NodeType get_type(const Node *node)
{
    return node->type;
}

size_t get_token_index(const Node *node)
{
    return node->token_index;
}

void set_token_index(Node *node, size_t token_index)
{
    node->token_index = token_index;
}

const Operator *get_op(const Node *node)
{
    return ((OperatorNode*)node)->op;
}

void set_op(Node *node, const Operator *op)
{
    ((OperatorNode*)node)->op = op;
}

size_t get_num_children(const Node *node)
{
    return ((OperatorNode*)node)->num_children;
}

Node *get_child(const Node *node, size_t index)
{
    return ((OperatorNode*)node)->children[index];
}

Node **get_child_addr(const Node *node, size_t index)
{
    return &((OperatorNode*)node)->children[index];
}

void set_child(Node *node, size_t index, Node *child)
{
    ((OperatorNode*)node)->children[index] = child;
}

const char *get_var_name(const Node *node)
{
    return ((VariableNode*)node)->var_name;
}

size_t get_id(const Node *node)
{
    return ((VariableNode*)node)->id;
}

void set_id(Node *node, size_t id)
{
    ((VariableNode*)node)->id = id;
}

double get_const_value(const Node *node)
{
    return ((ConstantNode*)node)->const_value;
}
