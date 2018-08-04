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
	
	/* For NTYPE_VARIABLE: */
	char *var_name;
	
	/* For NTYPE_CONSTANT: */
	void *const_value;
	
	/* For NTYPE_OPERATOR: */
	Operator *op;
	int num_children;
	struct Node *children[MAX_CHILDREN];

} Node;

Node get_variable_node(char *var_name);
Node get_constant_node(void *value);
Node get_operator_node(Operator *op);

bool tree_contains_variable(Node* node);
int list_variables(Node *tree, char **out_variables);
Node tree_copy(ParsingContext *ctx, Node *node);
int tree_substitute(ParsingContext *ctx, Node *dest_tree, Node *tree, char* var_name);
bool node_equals(ParsingContext *ctx, Node *a, Node *b);
bool tree_equals(ParsingContext *ctx, Node *a, Node *b);
void tree_replace(Node *destination, Node new_node);
