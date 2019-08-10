#pragma once
#include <stdbool.h>

#include "context.h"
#include "constants.h"
#include "operator.h"

typedef enum
{
    NTYPE_OPERATOR,
    NTYPE_CONSTANT,
    NTYPE_VARIABLE
} NodeType;

typedef struct Node
{
    NodeType type;
    // For NTYPE_VARIABLE:
    char *var_name;
    // For NTYPE_CONSTANT:
    void *const_value;
    // For NTYPE_OPERATOR:
    Operator *op;
    size_t num_children;
    struct Node **children;
} Node;

Node get_variable_node(char *var_name);
Node get_constant_node(void *value);
Node get_operator_node(Operator *op, size_t num_children);
void free_tree(Node *node);
void free_tree_preserved(Node *tree);
bool tree_contains_vars(Node *node);
size_t tree_get_var_instances(Node *tree, char *variable, Node **out_instances);
size_t tree_list_vars(Node *tree, char **out_variables);
size_t tree_substitute_var(ParsingContext *ctx, Node *tree, Node *tree_to_copy, char *var_name);
void tree_replace(Node *destination, Node new_node);
Node tree_copy(ParsingContext *ctx, Node *node);
bool node_equals(ParsingContext *ctx, Node *a, Node *b);
bool tree_equals(ParsingContext *ctx, Node *a, Node *b);
