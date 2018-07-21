#include <stdio.h>
#include <string.h>

#include "console_util.h"

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
