#include <string.h>

#include "node.h"
#include "memory.h"

Node get_node(NodeType type)
{
	Node res;
	res.type = type;
	return res;
}

/* Returns a new node of type NTYPE_VARIABLE and prepares its attributes */
Node get_variable_node(char *var_name)
{
	Node res = get_node(NTYPE_VARIABLE);
	res.var_name = var_name;
	return res;
}

/* Returns a new node of type NTYPE_CONSTANT and prepares its attributes */
Node get_constant_node(void *value)
{
	Node res = get_node(NTYPE_CONSTANT);
	res.const_value = value;
	return res;
}

/* Returns a new node of type NTYPE_OPERATOR and prepares its attributes */
Node get_operator_node(Operator *op)
{
	Node res = get_node(NTYPE_OPERATOR);
	res.op = op;
	res.num_children = 0;
	for (size_t i = 0; i < MAX_CHILDREN; i++) res.children[i] = NULL;
	return res;
}

/* Returns true iff variable node exists in tree
   False indicates save evaluation */
bool tree_contains_variable(Node* tree)
{
	if (tree == NULL) return false;
	
	switch (tree->type)
	{
		case NTYPE_CONSTANT:
			return false;
			
		case NTYPE_VARIABLE:
			return true;
			
		case NTYPE_OPERATOR:
			for (int i = 0; i < tree->num_children; i++) if (tree_contains_variable(tree->children[i])) return true;
			return false;
	}
	return false;
}

Node tree_copy(ParsingContext *ctx, Node *tree)
{
	Node res = *tree;
	
	switch (tree->type)
	{
		case NTYPE_OPERATOR:
			for (int i = 0; i < tree->num_children; i++)
			{
				res.children[i] = malloc(sizeof(Node));
				*(res.children[i]) = tree_copy(ctx, tree->children[i]);
			}
			break;
		
		case NTYPE_CONSTANT:
			res.const_value = malloc(ctx->value_size);
			for (size_t i = 0; i < ctx->value_size; i++)
			{
				*((char*)(res.const_value) + i) = *((char*)(tree->const_value) + i);
			}
			break;
			
		case NTYPE_VARIABLE:
			res.var_name = malloc((strlen(tree->var_name) + 1) * sizeof(char));
			strcpy(res.var_name, tree->var_name);
			break;
	}
	
	return res;
}

/*
Summary: Substitutes any occurence of a variable with certain name with a given subtree
*/
int tree_substitute(ParsingContext *ctx, Node *dest_tree, Node *tree, char *var_name)
{
	if (ctx == NULL || dest_tree == NULL || tree == NULL || var_name == NULL) return 0;
	
	int res = 0;
	
	switch(dest_tree->type)
	{
		case NTYPE_OPERATOR:
			for (int i = 0; i < dest_tree->num_children; i++)
			{
				res += tree_substitute(ctx, dest_tree->children[i], tree, var_name);
			}
			return res;
			
		case NTYPE_VARIABLE:
			if (strcmp(var_name, dest_tree->var_name) == 0)
			{
				*dest_tree = tree_copy(ctx, tree);
				return 1;
			}
			break;
			
		default:
			break;
	}
	
	return 0;
}

bool node_equals(ParsingContext *ctx, Node *a, Node *b)
{
	if (ctx == NULL || a == NULL || b == NULL) return false;
	
	if (a->type != b->type) return false;
	
	switch (a->type)
	{
		case NTYPE_OPERATOR:
			if (a->op != b->op) return false;
			if (a->num_children != b->num_children) return false;
			break;
			
		case NTYPE_CONSTANT:
			for (int i = 0; i < ctx->value_size; i++)
			{
				if (((char*)a->const_value)[i] != ((char*)b->const_value)[i]) return false;
			}
			break;
			
		case NTYPE_VARIABLE:
			if (strcmp(a->var_name, b->var_name) != 0) return false;
	}
	
	return true;
}

bool tree_equals(ParsingContext *ctx, Node *a, Node *b)
{
	if (ctx == NULL || a == NULL || b == NULL) return false;
	
	if (!node_equals(ctx, a, b)) return false;
	if (a->type == NTYPE_OPERATOR)
	{
		for (size_t i = 0; i < a->num_children; i++)
		{
			if (!tree_equals(ctx, a->children[i], b->children[i])) return false;
		}
	}
	return true;
}

/*
Summary: frees all child-trees and replaces root value-wise
*/
void tree_replace(Node *destination, Node new_node)
{
	if (destination == NULL) return;
	
	if (destination->type == NTYPE_OPERATOR)
	{
		for (int i = 0; i < destination->num_children; i++)
		{
			free_tree(destination->children[i]);
		}
	}
	
	*destination = new_node;
}
