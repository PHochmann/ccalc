#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "constants.h"
#include "operator.h"
#include "output.h"
#include "console_util.h"

void print_constant(ParsingContext *ctx, Node *node)
{
	if (node->type == NTYPE_CONSTANT)
	{
		char value[ctx->min_strbuf_length];
		ctx->to_string(node->const_value, value, ctx->min_strbuf_length);
		printf(CONST_COLOR "%s" COL_RESET, value);
	}
}

void print_variable(Node *node)
{
	if (node->type == NTYPE_VARIABLE)
	{
		printf(VAR_COLOR "%s" COL_RESET, node->var_name);
	}
}

void show_tree_rec(ParsingContext *ctx, Node *node, int layer, unsigned int vertical_lines, bool last_child)
{
	for (int i = 0; i < layer - 1; i++)
	{
		if (vertical_lines & (1 << i))
		{
			printf("│   "); // Bit at i=1
		}
		else
		{
			printf("    "); // Bit at i=0
		}
	}
	
	if (layer != 0)
	{
		if (!last_child)
		{
			printf("├── ");
		}
		else
		{
			printf("└── ");
		}
	}
	
	switch (node->type)
	{
		case NTYPE_OPERATOR:
			printf(OP_COLOR "%s" COL_RESET "\n", node->op->name);
			for (int i = 0; i < node->num_children; i++)
			{
				bool is_last = i == node->num_children - 1;
				show_tree_rec(
					ctx,
					node->children[i], layer + 1, is_last ? vertical_lines : vertical_lines | (1 << layer),
					is_last);
			}
			break;
			
		case NTYPE_CONSTANT:
			print_constant(ctx, node);
			printf("\n");
			break;
			
		case NTYPE_VARIABLE:
			print_variable(node);
			printf("\n");
			break;
	}
}

/*
Summary: Draws coloured tree of node to stdout
*/
void show_tree(ParsingContext *ctx, Node *node)
{
	if (ctx == NULL || node == NULL) return;
	show_tree_rec(ctx, node, 0, 0, true);
}

void inline_tree_rec(ParsingContext *ctx, Node *node, bool needs_p)
{
	switch (node->type)
	{
		case NTYPE_CONSTANT:
			print_constant(ctx, node);
			break;
			
		case NTYPE_VARIABLE:
			print_variable(node);
			break;
			
		case NTYPE_OPERATOR:
			if (needs_p) printf("(");
			bool l_needs = false;
			bool r_needs = false;
			
			switch (node->op->placement)
			{
				case OP_PLACE_PREFIX:
					printf("%s", node->op->name);
					if (node->op->arity != 0)
					{
						inline_tree_rec(ctx, node->children[0], true);
					}
					break;
					
				case OP_PLACE_POSTFIX:
					if (node->op->arity != 0)
					{
						inline_tree_rec(ctx, node->children[0], true);
					}
					printf("%s", node->op->name);
					break;
					
				case OP_PLACE_FUNCTION:
					printf("%s(", node->op->name);
					for (size_t i = 0; i < node->num_children; i++)
					{
						inline_tree_rec(ctx, node->children[i], false);
						if (i != node->num_children - 1) printf(", ");
					}
					printf(")");
					break;
					
				case OP_PLACE_INFIX:
					if (node->children[0]->type == NTYPE_OPERATOR
						&& (node->children[0]->op->placement == OP_PLACE_POSTFIX
							|| node->children[0]->op->precedence < node->op->precedence
							|| (node->children[0]->op->precedence == node->op->precedence
								&& node->op->assoc == OP_ASSOC_RIGHT))) l_needs = true;

					if (node->children[1]->type == NTYPE_OPERATOR
						&& node->op->arity != 0
						&& (node->children[1]->op->precedence < node->op->precedence
							|| node->children[1]->op->placement == OP_PLACE_PREFIX
							|| (node->children[1]->op->precedence == node->op->precedence
								&& node->op->assoc == OP_ASSOC_LEFT))) r_needs = true;
				
					inline_tree_rec(ctx, node->children[0], l_needs);
					printf("%s", node->op->name);
					inline_tree_rec(ctx, node->children[1], r_needs);
			}
			
			if (needs_p) printf(")");
			break;
	}
}

/*
Summary: Prints expression of node to stdout
*/
void inline_tree(ParsingContext *ctx, Node *node)
{
	if (ctx == NULL || node == NULL) return;
	inline_tree_rec(ctx, node, false);
}
