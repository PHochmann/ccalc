#include <stdio.h>
#include <stdbool.h>

#include "constants.h"
#include "output.h"

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
	
	char value[ctx->min_strbuf_length];
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
			ctx->to_string(node->const_value, value, ctx->min_strbuf_length);
			printf(CONST_COLOR "%s" COL_RESET "\n", value);
			break;
			
		case NTYPE_VARIABLE:
			printf(VAR_COLOR "%s" COL_RESET "\n", node->var_name);
			break;
	}
}

/*
Summary: Draws coloured tree of node to stdout
*/
void show_tree(ParsingContext *ctx, Node *node)
{
	show_tree_rec(ctx, node, 0, 0, true);
}

char* perr_to_string(ParserError perr)
{
	switch (perr)
	{
		case PERR_SUCCESS:
			return "SUCCESS";
		case PERR_NOT_INIT:
			return "NOT INITIALIZED";
		case PERR_MAX_TOKENS_EXCEEDED:
			return "MAX TOKENS EXCEEDED";
		case PERR_STACK_EXCEEDED:
			return "STACK EXCEEDED";
		case PERR_UNEXPECTED_TOKEN:
			return "UNEXPECTED TOKEN";
		case PERR_UNEXPECTED_OPENING_PARENTHESIS:
			return "UNEXPECTED OPENING PARENTHESIS";
		case PERR_UNEXPECTED_CLOSING_PARENTHESIS:
			return "UNEXPECTED CLOSING PARENTHESIS";
		case PERR_UNEXPECTED_DELIMITER:
			return "UNEXPECTED DELIMITER";
		case PERR_MISSING_OPERATOR:
			return "UNEXPECTED OPERAND";
		case PERR_MISSING_OPERAND:
			return "MISSING OPERAND";
		case PERR_EXCEEDED_MAX_CHILDREN:
			return "TOO MANY OPERANDS";
		case PERR_EMPTY:
			return "EMPTY EXPRESSION";
		default:
			return "UNKNOWN ERROR";
	}
}

void print_ops(ParsingContext *ctx)
{
	for (int i = 0; i < ctx->num_ops; i++)
	{
		printf(OP_COLOR "%s" COL_RESET " ", ctx->operators[i].name);
	}
	printf("\n(%d available operators)\n\n", ctx->num_ops);
}