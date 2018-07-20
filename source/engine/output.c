#include <stdio.h>
#include <string.h>
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

char* opplace_to_string(Op_Placement place)
{
	switch (place)
	{
		case OP_PLACE_PREFIX:
			return "PREFIX";
		case OP_PLACE_INFIX:
			return "INFIX";
		case OP_PLACE_POSTFIX:
			return "POSTFIX";
		case OP_PLACE_FUNCTION:
			return "FUNCTION";
		default:
			return "UNKNOWN";
	}
}

char* perr_to_string(ParserError perr)
{
	switch (perr)
	{
		case PERR_SUCCESS:
			return "SUCCESS";
		case PERR_NOT_INIT:
			return "PARSER NOT INITIALIZED";
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
	printf("\n(%d available operators. Further help with 'help <op>' or 'help all')\n", ctx->num_ops);
	printf("\nUse 'ans' as a variable to refer to previous result.\n\n");
}

void print_op_info(ParsingContext *ctx, char *name)
{
	int res = 0;
	int IDs[ctx->num_ops];
	
	for (int i = 0; i < ctx->num_ops; i++)
	{
		if (strcmp(ctx->operators[i].name, name) == 0 || strcmp(name, "all") == 0)
		{
			IDs[res++] = i;
		}
	}
	
	int header = 5;
	
	char** cells = malloc(sizeof(char*) * (res + 1) * header);
	for (int i = header; i < (res + 1) * header; i++) cells[i] = malloc(sizeof(char) * 10);
	
	cells[0] = "ID";
	cells[1] = "Name";
	cells[2] = "Arity";
	cells[3] = "Placement";
	cells[4] = "Precedence";
	for (int i = 0; i < res; i++)
	{
		sprintf(cells[header * (i + 1) + 0], "%d", IDs[i]);
		strcpy(cells[header * (i + 1) + 1], ctx->operators[IDs[i]].name);
		sprintf(cells[header * (i + 1) + 2], "%d", ctx->operators[IDs[i]].arity);
		strcpy(cells[header * (i + 1) + 3], ctx->operators[IDs[i]].arity != 0 ? opplace_to_string(ctx->operators[IDs[i]].placement) : "CONSTANT");
		sprintf(cells[header * (i + 1) + 4], "%d", ctx->operators[IDs[i]].precedence);
	}
	print_table(res + 1, header, cells);
	for (int i = header; i < (res + 1) * header; i++) free(cells[i]);
	free(cells);
}

void print_padded(char *string, int total_length)
{
	for (int i = strlen(string); i < total_length; i++) printf(" ");
	printf("%s", string);
}

void print_repeated(char* string, int amount)
{
	for (int i = 0; i < amount; i++) printf("%s", string);
}

void print_table(int num_rows, int num_cols, char** cells)
{
	int width[num_cols];
	for (int i = 0; i < num_cols; i++) width[i] = 0;
	
	// Calculate col width
	for (int i = 0; i < num_rows; i++)
	{
		for (int j = 0; j < num_cols; j++)
		{
			int len = strlen(cells[i * num_cols + j]);
			if (width[j] < len) width[j] = len;
		}
	}
	// - - -
	
	// Print top border
	printf("┌");
	for (int i = 0; i < num_cols; i++)
	{
		print_repeated("─", width[i] + 2);
		if (i != num_cols - 1) printf("┬");
	}
	printf("┐\n");
	// - - -
	
	// Print cells
	for (int i = 0; i < num_rows; i++)
	{
		if (i == 1 && num_rows > 1) // Print table head border
		{
			printf("├");
			for (int i = 0; i < num_cols; i++)
			{
				print_repeated("─", width[i] + 2);
				if (i != num_cols - 1) printf("┼");
			}
			printf("┤\n");
		}
		
		printf("│");
		for (int j = 0; j < num_cols; j++)
		{
			printf(" ");
			print_padded(cells[i * num_cols + j], width[j]);
			printf(" │");
		}
		printf("\n");
	}
	// - - -
	
	// Print bottom border
	printf("└");
	for (int i = 0; i < num_cols; i++)
	{
		print_repeated("─", width[i] + 2);
		if (i != num_cols - 1) printf("┴");
	}
	printf("┘\n\n");
	// - - -
}