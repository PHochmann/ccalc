#pragma once
#include "node.h"

#define LISTENERERR_SUCCESS                0
#define LISTENERERR_VARIABLE_ENCOUNTERED -50

typedef int ListenerError;
typedef ListenerError (*TreeListener)(const Operator *op, size_t num_children, const double *children, double *out);
typedef double (*OpEval)(size_t num_children, const Node * const *children);

// Data handling
bool tree_equals(const Node *a, const Node *b);
Node *tree_copy(const Node *node);
void tree_replace(Node **tree_to_replace, Node *tree_to_insert);
void tree_replace_by_list(Node **parent, size_t child_to_replace, NodeList list);

// Helper and convenience functions
size_t count_all_variable_nodes(const Node *tree);
size_t get_variable_nodes(const Node * const *tree, const char *var_name, size_t buffer_size, Node ***out_instances);
size_t list_variables(Node *tree, size_t buffer_size, const char **out_vars, bool *out_sufficient_buff);
void tree_copy_IDs(Node *tree, size_t num_vars, const char **vars);
Node **find_op(const Node * const *tree, const Operator *op);

// Traversal
size_t replace_variable_nodes(Node **tree, const Node *tree_to_copy, const char *var_name);
ListenerError tree_reduce(const Node *tree, TreeListener listener, double *out, const Node **out_errnode);
ListenerError tree_reduce_constant_subtrees(Node **tree, TreeListener listener, const Node **out_errnode);
void tree_reduce_ops(Node **tree, const Operator *op, OpEval eval);
