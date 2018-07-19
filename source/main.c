#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "engine/constants.h"
#include "engine/node.h"
#include "engine/output.h"
#include "engine/tokenizer.h"
#include "engine/parser.h"

#include "arithmetic/arith.h"

#define VERSION "0.0.4"
#define MAX_LINE_LENGTH 256

ParsingContext ctx;
Node *ans;

void _exit()
{
	uninit_parser();
}

void parse_input(char *input)
{
	Node *res;
	ParserError perr = parse_node(&ctx, input, &res);
	
	if (perr == PERR_SUCCESS)
	{
		if (ans != NULL) tree_substitute(&ctx, &res, ans, "ans", true);
		show_tree(&ctx, res);
		
		if (!tree_contains_variable(res))
		{
			double eval = arith_eval(res);
			char result_str[ctx.min_strbuf_length];
			ctx.to_string((void*)(&eval), result_str, ctx.min_strbuf_length);
			printf("= %s\n", result_str);
		}
		
		if (ans != NULL) tree_free(ans);
		ans = res;
	}
	else
	{
		printf("Error: %s\n", perr_to_string(perr));
	}
	
	printf("\n");
}

int main(int argc, char *argv[])
{
	atexit(_exit);

	ctx = arith_get_ctx();
	ans = NULL;
	init_parser();
	
	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			parse_input(argv[i]);
		}
	}
	else
	{
		printf("Calculator %s (c) 2018, Philipp Hochmann\n(Commands: help)\n\n", VERSION);
		char input[MAX_LINE_LENGTH];
		
		while (true)
		{
			printf("> ");
			if (fgets(input, MAX_LINE_LENGTH, stdin) != NULL)
			{
				input[strlen(input) - 1] = '\0';
				
				if (strcmp(input, "exit") == 0) break;
				if (begins_with("help", input))
				{
					if (strlen(input) > 5)
					{
						print_op_info(&ctx, input + 5);
					}
					else
					{
						print_ops(&ctx);
					}
					continue;
				}
				
				parse_input(input);
			}
			else
			{
				break;
			}
		}

	}
	
	return 0;
}