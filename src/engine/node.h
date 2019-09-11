#pragma once
#include <stdbool.h>
#include "context.h"
#include "operator.h"

extern const size_t MAX_TREE_SEARCH_STACK_SIZE;
extern const size_t MAX_VAR_COUNT;

typedef enum
{
    NTYPE_OPERATOR,
    NTYPE_CONSTANT,
    NTYPE_VARIABLE
} NodeType;

/*
Summary: Tree consists of nodes that are either operators, constants or variables
    Operators are usually inner nodes (exceptions: operator-constants or zero-arity functions)
    Constants are leaf nodes operators work upon
    Everything else is a variable that needs to be substituted before evaluating
*/
typedef struct Node
{
    NodeType type;          // Operator, constant or variable
    char *var_name;         // For variables
    void *const_value;      // For constant, on heap
    Operator *op;           // For operator, points to operator in ctx
    size_t num_children;    // Size of children buffer
    struct Node **children; // On heap, can not be flexible array member due to tree_replace 
} Node;

Node get_variable_node(char *var_name);
Node get_constant_node(void *value);
Node get_operator_node(Operator *op, size_t num_children);
void free_tree(Node *node);
void free_tree_preserved(Node *tree);
size_t tree_count_vars(Node *node);
size_t tree_get_var_instances(Node *tree, char *variable, Node **out_instances);
size_t tree_list_vars(Node *tree, char **out_variables);
size_t tree_substitute_var(ParsingContext *ctx, Node *tree, Node *tree_to_copy, char *var_name);
void tree_replace(Node *destination, Node new_node);
Node tree_copy(ParsingContext *ctx, Node *node);
bool node_equals(ParsingContext *ctx, Node *a, Node *b);
bool tree_equals(ParsingContext *ctx, Node *a, Node *b);
