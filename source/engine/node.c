#include <string.h>

#include "node.h"

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

/* Summary: Calls free() on each node within tree, including variable's names and constant's values */
void tree_free(Node *tree)
{
	switch (tree->type)
	{
		case NTYPE_OPERATOR:
			for (int i = 0; i < tree->num_children; i++)
			{
				tree_free(tree->children[i]);
			}
			break;
			
		case NTYPE_CONSTANT:
			free(tree->const_value);
			break;
			
		case NTYPE_VARIABLE:
			free(tree->var_name);
			break;
	}
	
	free(tree);
}

/*
Summary: Substitutes any occurence of a variable with certain name with a given subtree
When copy is set to true, 'tree' is copied each time. When set to false, a pointer is bent to 'tree'.
Double frees might occur when using free_tree
*/
int tree_substitute(ParsingContext *ctx, Node **dest_tree, Node *tree, char* var_name, bool copy)
{
	if (dest_tree == NULL || *dest_tree == NULL) return 0;
	
	int res = 0;
	
	switch((*dest_tree)->type)
	{
		case NTYPE_OPERATOR:
			for (int i = 0; i < (*dest_tree)->num_children; i++)
			{
				res += tree_substitute(ctx, &(*dest_tree)->children[i], tree, var_name, copy);
			}
			return res;
			
		case NTYPE_VARIABLE:
			if (strcmp(var_name, (*dest_tree)->var_name) == 0)
			{
				if (copy)
				{
					**dest_tree = tree_copy(ctx, tree);
				}
				else
				{
					*dest_tree = tree;
				}
				return 1;
			}
			break;
			
		default:
			break;
	}
	
	return 0;
}