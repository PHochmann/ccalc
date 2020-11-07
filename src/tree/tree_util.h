#pragma once
#include "node.h"

typedef bool (*Evaluation)(const Operator *op, size_t num_children, const double *children, double *out);

// Data handling
bool tree_equals(const Node *a, const Node *b);
Node *tree_copy(const Node *node);
void tree_replace(Node **tree_to_replace, Node *tree_to_insert);
void tree_replace_by_list(Node **parent, size_t child_to_replace, NodeList list);

// Helper and convenience functions
size_t count_all_variable_nodes(const Node *tree);
size_t get_variable_nodes(const Node **tree, const char *var_name, Node ***out_instances);
size_t list_variables(const Node *tree, size_t buffer_size, const char **out_variables);
Node **find_op(const Node **tree, const Operator *op);
size_t replace_variable_nodes(Node **tree, const Node *tree_to_copy, const char *var_name);
bool tree_reduce(const Node *tree, Evaluation eval, double *out);
void replace_constant_subtrees(Node **tree, Evaluation eval, size_t num_dont_reduce, const Operator **dont_reduce);
