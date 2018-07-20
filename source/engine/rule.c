#include <stdbool.h>
#include <string.h>

#include <stdio.h>

#include "constants.h"
#include "rule.h"

/*
Summary: Tries to construct matching in root node
*/
bool get_matching(ParsingContext *ctx, Node *tree, Node *pattern, Matching *result)
{
	char *mapped_vars[MAPPING_MAX_VAR_COUNT];
	Node *mapped_nodes[MAPPING_MAX_VAR_COUNT];
	int num_mapped_vars = 0;
	
	Node *tree_stack[MAX_STACK_SIZE];
	Node *pattern_stack[MAX_STACK_SIZE];
	int num_stack = 0;
	
	tree_stack[0] = tree;
	pattern_stack[0] = pattern;
	num_stack = 1;
	
	while (num_stack != 0)
	{
		Node *curr_pattern_n = pattern_stack[num_stack - 1];
		Node *curr_tree_n = tree_stack[num_stack - 1];
		num_stack--;
		
		bool found = false;
		switch (curr_pattern_n->type)
		{
			// 1. Check if variable is bound, if it is, check it. Otherwise, bind.
			case NTYPE_VARIABLE:
				for (int i = 0; i < num_mapped_vars; i++)
				{
					if (strcmp(mapped_vars[i], curr_pattern_n->var_name) == 0) // Already bound
					{
						found = true;
						if (!tree_equals(ctx, mapped_nodes[i], curr_tree_n)) return false;
					}
				}
				
				if (!found)
				{
					mapped_vars[num_mapped_vars] = malloc(sizeof(char) * strlen(curr_pattern_n->var_name));
					strcpy(mapped_vars[num_mapped_vars], curr_pattern_n->var_name);
					mapped_nodes[num_mapped_vars] = malloc(sizeof(Node));
					*(mapped_nodes[num_mapped_vars]) = tree_copy(ctx, curr_tree_n);
					num_mapped_vars++;
				}
				
				break;
				
			// 2. Check constants for equality
			case NTYPE_CONSTANT:
				if (!node_equals(ctx, curr_pattern_n, curr_tree_n)) return false;
				break;
				
			// 3. Check operands for equality and add children on stack
			case NTYPE_OPERATOR:
				if (!node_equals(ctx, curr_pattern_n, curr_tree_n)) return false;
				for (size_t i = 0; i < curr_pattern_n->num_children; i++)
				{
					tree_stack[num_stack + i] = curr_tree_n->children[i];
					pattern_stack[num_stack + i] = curr_pattern_n->children[i];
				}
				num_stack += curr_pattern_n->num_children;
				break;
		}
		
	}
	
	// We successfully found matching! Construct it:
	result->matched_tree = tree;
	result->mapped_vars = malloc(sizeof(char*) * num_mapped_vars);
	result->mapped_nodes = malloc(sizeof(Node*) * num_mapped_vars);
	
	for (int i = 0; i < num_mapped_vars; i++)
	{
		result->mapped_vars[i] = mapped_vars[i];
		result->mapped_nodes[i] = mapped_nodes[i];
	}
	result->num_mapped = num_mapped_vars;
	return true;
}

/*
Summary: Looks for matching in tree, i.e. tries to construct matching in each node until matching is found (Top-Down)
*/
bool find_matching(ParsingContext *ctx, Node *tree, Node *pattern, Matching *result)
{
	if (get_matching(ctx, tree, pattern, result)) return true;
	
	if (tree->type == NTYPE_OPERATOR)
	{
		for (int i = 0; i < tree->num_children; i++)
		{
			if (find_matching(ctx, tree->children[i], pattern, result)) return true;
		}
	}
	
	return false;
}

void free_matching(Matching matching)
{
	for (int i = 0; i < matching.num_mapped; i++)
	{
		free(matching.mapped_vars[i]);
		free(matching.mapped_nodes[i]);
	}
	free(matching.mapped_vars);
	free(matching.mapped_nodes);
}