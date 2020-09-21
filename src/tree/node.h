#pragma once
#include <stdbool.h>
#include "operator.h"

/*
Trees consist of nodes that are either operators, constants or variables.
Operators are usually inner nodes (exception: zero-arity functions).
Constants and variables are leaf nodes.
*/

typedef double ConstantType;

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
    size_t id; // For easier lookup
    char var_name[];
} VariableNode;

typedef struct {
    Node base;
    ConstantType const_value;
} ConstantNode;

typedef struct {
    Node base;
    const Operator *op;        // Points to operator in context
    size_t num_children; // Size of children buffer
    Node *children[];
} OperatorNode;

// Not to be confused with struct ListNode ;)
typedef struct {
    size_t size;
    const Node **nodes;
} NodeList;

// Memory
Node *malloc_variable_node(const char *var_name, size_t id);
Node *malloc_constant_node(ConstantType value);
Node *malloc_operator_node(const Operator *op, size_t num_children);
void free_tree(Node *tree);

// Accessors
NodeType get_type(const Node *node);
const Operator *get_op(const Node *node);
size_t get_num_children(const Node *node);
Node *get_child(const Node *node, size_t index);
Node **get_child_addr(const Node *node, size_t index);
void set_child(Node *node, size_t index, Node *child);
char *get_var_name(const Node *node);
size_t get_id(const Node *node);
void set_id(Node *node, size_t id);
ConstantType get_const_value(const Node *node);
