#pragma once
#include <stdbool.h>
#include "operator.h"

/*
Trees consist of nodes that are either operators, constants or variables.
Operators are usually inner nodes (exception: zero-arity functions).
Constants and variables are leaf nodes.
*/

typedef enum {
    NTYPE_OPERATOR,
    NTYPE_CONSTANT,
    NTYPE_VARIABLE
} NodeType;

typedef struct {
    NodeType type;
} Node;

typedef struct {
    Node base;
    char var_name[];
} VariableNode;

typedef double ConstantType;

typedef struct {
    Node base;
    ConstantType const_value;
} ConstantNode;

typedef struct {
    Node base;
    Operator *op;        // Points to operator in context
    size_t num_children; // Size of children buffer
    Node *children[];
} OperatorNode;

// Memory
Node *malloc_variable_node(char *var_name);
Node *malloc_constant_node(ConstantType value);
Node *malloc_operator_node(Operator *op, size_t num_children);
void free_tree(Node *tree);

// Accessors
NodeType get_type(Node *node);
Operator *get_op(Node *node);
size_t get_num_children(Node *node);
Node *get_child(Node *node, size_t index);
Node **get_child_addr(Node *node, size_t index);
void set_child(Node *node, size_t index, Node *child);
char *get_var_name(Node *node);
ConstantType get_const_value(Node *node);

// Data handling
Node *tree_equals(Node *a, Node *b);
Node *tree_copy(Node *node);
void tree_replace(Node **tree_to_replace, Node *tree_to_insert);

// Helper and convenience functions
size_t count_variables(Node *tree);
size_t count_variables_distinct(Node *tree);
size_t get_variable_nodes(Node **tree, char *var_name, Node ***out_instances);
size_t count_variable_nodes(Node *tree, char *var_name);
size_t list_variables(Node *tree, char **out_variables);
size_t replace_variable_nodes(Node **tree, Node *tree_to_copy, char *var_name);
