#include <stdbool.h>
#include <string.h>

#include "str_util.h"
#include "rule.h"
#include "constants.h"
#include "parser.h"
#include "node.h"

#define VAR_PREFIX "var_"
#define CONST_PREFIX "const_"

/*
Summary: Tries to match 'tree' against 'pattern' (only in root)
Returns: true, if matching is found, false if NULL-pointers given in arguments or no matching found
*/
bool get_matching(ParsingContext *ctx, Node *tree, Node *pattern, Matching *out_matching)
{
	if (ctx == NULL || tree == NULL || pattern == NULL || out_matching == NULL) return false;
	
	char *mapped_vars[MAX_VAR_COUNT];
	Node *mapped_nodes[MAX_VAR_COUNT];
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
					// Check special rules
					if (begins_with(CONST_PREFIX, curr_pattern_n->var_name) && curr_tree_n->type != NTYPE_CONSTANT) return false;
					if (begins_with(VAR_PREFIX, curr_pattern_n->var_name) && curr_tree_n->type != NTYPE_VARIABLE) return false;
					
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
	out_matching->matched_tree = tree;
	out_matching->mapped_vars = malloc(sizeof(char*) * num_mapped_vars);
	out_matching->mapped_nodes = malloc(sizeof(Node*) * num_mapped_vars);
	
	for (int i = 0; i < num_mapped_vars; i++)
	{
		out_matching->mapped_vars[i] = mapped_vars[i];
		out_matching->mapped_nodes[i] = mapped_nodes[i];
	}
	out_matching->num_mapped = num_mapped_vars;
	return true;
}

/*
Summary: Looks for matching in tree, i.e. tries to construct matching in each node until matching is found (Top-Down)
*/
bool find_matching(ParsingContext *ctx, Node *tree, Node *pattern, Matching *out_matching)
{
	if (ctx == NULL || tree == NULL || pattern == NULL || out_matching == NULL) return false;
	
	if (get_matching(ctx, tree, pattern, out_matching)) return true;
	
	if (tree->type == NTYPE_OPERATOR)
	{
		for (int i = 0; i < tree->num_children; i++)
		{
			if (find_matching(ctx, tree->children[i], pattern, out_matching)) return true;
		}
	}
	
	return false;
}

/*
Summary: Constructs rule from pattern-string (before) and transformation-string (after)
Returns: true, if rule was successfully constructed from supplied strings, false otherwise
*/
bool parse_rule(ParsingContext *ctx, char *before, char *after, RewriteRule *out_rule)
{
	if (ctx == NULL || before == NULL || after == NULL || out_rule == NULL) return false;
	
	Node *before_n;
	Node *after_n;
	
	if (parse_node(ctx, before, &before_n) != PERR_SUCCESS) return false;
	if (parse_node(ctx, after, &after_n) != PERR_SUCCESS) return false;
	
	out_rule->context = ctx;
	out_rule->before = before_n;
	out_rule->after = after_n;
	
	return true;
}

void transform_by_rule(RewriteRule *rule, Matching *matching)
{
	if (rule == NULL || matching == NULL) return;
	
	Node transformed = tree_copy(rule->context, rule->after);
	
	for (size_t i = 0; i < matching->num_mapped; i++)
	{
		tree_substitute(rule->context,
			&transformed,
			matching->mapped_nodes[i],
			matching->mapped_vars[i]);
	}
	
	tree_replace(matching->matched_tree, transformed);
}

bool apply_rule(Node *tree, RewriteRule *rule)
{
	Matching matching;
	// Try to find matching in tree with pattern specified in rule
	if (!find_matching(rule->context, tree, rule->before, &matching)) return false;
	// If matching is found, transform tree with it
	transform_by_rule(rule, &matching);
	return true;
}

/*
Summary: Tries to apply rules in round-robin fashion until no rule can be applied any more
Params
	max_iterations: maximal number of times the set is iterated, -1 for no cap (this makes non-termination possible)
Returns: Number of successful appliances
*/
int apply_ruleset(Node *tree, RewriteRule *rules, int num_rules, int max_iterations)
{
	int i = 0;
	int res = 0;
	
	while (i < max_iterations || max_iterations == -1)
	{
		bool applied_flag = false;
		
		for (int j = 0; j < num_rules; j++)
		{
			if (apply_rule(tree, &rules[j]))
			{
				res++;
				applied_flag = true;
			}
		}
		
		i++;
		if (!applied_flag) return res;
	}
	
	return res;
}
