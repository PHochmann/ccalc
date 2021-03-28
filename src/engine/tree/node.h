#pragma once
#include <stdbool.h>
#include "operator.h"

/*
Trees consist of nodes that are either operators, constants or variables.
Operators are usually inner nodes (exception: zero-arity functions).
Constants and variables are leaf nodes.
*/

#define MAX_CHILDREN 16

// Defined in node.c
typedef struct Node Node;

typedef enum {
    NTYPE_OPERATOR,
    NTYPE_CONSTANT,
    NTYPE_VARIABLE
} NodeType;

// Not to be confused with struct ListNode ;)
typedef struct {
    size_t size;
    const Node **nodes;
} NodeList;

// Memory
Node *malloc_variable_node(const char *var_name, size_t id, size_t tok_index);
Node *malloc_constant_node(double value, size_t tok_index);
Node *malloc_operator_node(const Operator *op, size_t num_children, size_t tok_index);
void free_tree(Node *tree);

// Accessors
NodeType get_type(const Node *node);
size_t get_token_index(const Node *node);
void set_token_index(Node *node, size_t token_index);
const Operator *get_op(const Node *node);
void set_op(Node *node, const Operator *op);
size_t get_num_children(const Node *node);
Node *get_child(const Node *node, size_t index);
Node **get_child_addr(const Node *node, size_t index);
void set_child(Node *node, size_t index, Node *child);
const char *get_var_name(const Node *node);
size_t get_id(const Node *node);
void set_id(Node *node, size_t id);
double get_const_value(const Node *node);
