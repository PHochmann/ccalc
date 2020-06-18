#pragma once
#include "node.h"

// Data handling
Node *tree_compare(Node *a, Node *b);
Node *tree_copy(Node *node);
void tree_replace(Node **tree_to_replace, Node *tree_to_insert);

// Helper and convenience functions
size_t count_variables(Node *tree);
size_t count_variables_distinct(Node *tree);
size_t get_variable_nodes(Node **tree, char *var_name, Node ***out_instances);
size_t count_variable_nodes(Node *tree, char *var_name);
size_t list_variables(Node *tree, char **out_variables);
size_t replace_variable_nodes(Node **tree, Node *tree_to_copy, char *var_name);
bool reduce(Node *tree, Evaluation eval, ConstantType *out);
ConstantType convenient_reduce(Node *tree, Evaluation eval);
void replace_constant_subtrees(Node **tree, Evaluation eval);

// Todo: refactor this
size_t replace_variable_nodes_by_list(Node **tree, NodeList list_of_nodes_to_copy, char *var_name, char id);
void tree_replace_by_list(Node **parent, size_t child_to_replace, NodeList list);
