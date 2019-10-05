#pragma once
#include <stdbool.h>
#include "operator.h"

// Maximal amount of variables nodes allowed in tree
#define MAX_VAR_COUNT 20

typedef double ConstantType;

/*
    Trees consist of nodes that are either operators, constants or variables
    Operators are usually inner nodes
        Exceptions: constant-operators (like pi or e) that are modeled as infix-operators with arity 0
            or zero-arity functions)
    Constants are leaf nodes operators work upon
    Everything else is a variable that needs to be substituted before evaluating
*/

typedef enum
{
    NTYPE_OPERATOR,
    NTYPE_CONSTANT,
    NTYPE_VARIABLE
} NodeType;

typedef NodeType* Node;

struct VariableNode_
{
    NodeType type;
    char var_name[];
};

struct ConstantNode_
{
    NodeType type;
    ConstantType const_value;
};

struct OperatorNode_
{
    NodeType type;
    Operator *op;        // Points to operator in context
    size_t num_children; // Size of children buffer
    Node children[];
};

typedef struct VariableNode_* VariableNode;
typedef struct ConstantNode_* ConstantNode;
typedef struct OperatorNode_* OperatorNode;

// Memory
Node malloc_variable_node(char *var_name);
Node malloc_constant_node(ConstantType value);
Node malloc_operator_node(Operator *op, size_t num_children);
void free_tree(Node tree);

// Accessors
Operator *get_op(Node node);
NodeType get_type(Node node);
size_t get_num_children(Node node);
Node get_child(Node node, size_t index);
Node *get_child_addr(Node node, size_t index);
void set_child(Node node, size_t index, Node child);
char *get_var_name(Node node);
ConstantType get_const_value(Node node);

// Data handling
bool tree_equals(Node a, Node b);
Node tree_copy(Node node);
void tree_replace(Node *tree_to_replace, Node tree_to_insert);

// Helper and convenience functions
size_t count_variables(Node tree);
size_t count_variables_distinct(Node tree);
size_t get_variable_nodes(Node *tree, char *var_name, Node **out_instances);
size_t count_variable_nodes(Node tree, char *var_name);
size_t list_variables(Node tree, char **out_variables);
void replace_variable_nodes(Node *tree, Node tree_to_copy, char *var_name);
