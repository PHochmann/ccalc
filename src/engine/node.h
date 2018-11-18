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

bool tree_contains_variable(Node* node);
int tree_get_variable_instances(Node *tree, char *variable, Node *out_instances[MAX_VAR_COUNT]);
int tree_list_variables(Node *tree, char *out_variables[MAX_VAR_COUNT]);
int tree_substitute_variable(ParsingContext *ctx, Node *tree, Node *tree_to_copy, char* var_name);
void tree_replace(Node *destination, Node new_node);
Node tree_copy(ParsingContext *ctx, Node *node);
bool node_equals(ParsingContext *ctx, Node *a, Node *b);
bool tree_equals(ParsingContext *ctx, Node *a, Node *b);
