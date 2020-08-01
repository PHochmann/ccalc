#pragma once
#include "node.h"

typedef bool (*Evaluation)(Operator *op, size_t num_children, ConstantType *children, ConstantType *out);

// Data handling
Node *tree_compare(Node *a, Node *b);
Node *tree_copy(Node *node);
void tree_replace(Node **tree_to_replace, Node *tree_to_insert);
void tree_replace_by_list(Node **parent, size_t child_to_replace, NodeList list);

// Helper and convenience functions
size_t count_variables(Node *tree);
size_t get_variable_nodes(Node **tree, char *var_name, Node ***out_instances);
size_t count_variable_nodes(Node *tree, char *var_name);
size_t list_variables(Node *tree, size_t buffer_size, char **out_variables);
Node **find_op(Node **tree, Operator *op);
size_t replace_variable_nodes(Node **tree, Node *tree_to_copy, char *var_name);
size_t replace_variable_nodes_by_list(Node **tree, NodeList nodes_to_copy, char *var_name);
bool tree_reduce(Node *tree, Evaluation eval, ConstantType *out);
void replace_constant_subtrees(Node **tree, Evaluation eval, size_t num_dont_reduce, Operator **dont_reduce);
