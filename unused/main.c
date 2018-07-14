#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "engine/constants.h"
#include "engine/node.h"
#include "engine/output.h"

#include "arithmetic/arith.h"
#include "propositional/prop.h"

#define VERSION "0.0.2"
#define MAX_LINE_LENGTH 256

ParsingContext arith_ctx;
ParsingContext prop_ctx;
ParsingContext *curr_ctx;

Node *ans;

void _exit()
{
	uninit_parser();
}

void parse_input(char *input)
{
	Node *res;
	ParserError perr = parse_node(curr_ctx, input, &res);
	
	if (perr == PERR_SUCCESS)
	{
		if (ans != NULL) tree_substitute(curr_ctx, res, ans, "ans");
		show_tree(curr_ctx, res);
		
		// Evaluate result if tree is constant
		if (!tree_contains_variable(res))
		{
			void *eval;
			eval = malloc(curr_ctx->value_size);
			
			if (curr_ctx == &arith_ctx)
			{
				*(double*)eval = arith_eval(res);
			}
			else
			{
				*(bool*)eval = prop_eval(res);
			}
			
			char result_str[curr_ctx->min_strbuf_length];
			curr_ctx->to_string(eval, result_str, curr_ctx->min_strbuf_length);
			printf("= %s\n", result_str);
		}
		// - - -
		
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

	arith_ctx = arith_get_ctx();
	prop_ctx = prop_get_ctx();
	curr_ctx = &arith_ctx;
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
		printf("Calculator %s (c) 2018, Philipp Hochmann\n(Commands: help, arith, prop) \n\n", VERSION);
		char input[MAX_LINE_LENGTH];
		
		while (true)
		{
			printf("> ");
			if (fgets(input, MAX_LINE_LENGTH, stdin) != NULL)
			{
				input[strlen(input) - 1] = '\0';
				
				if (strcmp(input, "exit") == 0) return 0;
				if (strcmp(input, "arith") == 0)
				{
					curr_ctx = &arith_ctx;
					printf("Switched to arithmetic mode\n\n");
					if (ans != NULL) tree_free(ans);
					ans = NULL;
					continue;
				}
				if (strcmp(input, "prop") == 0)
				{
					curr_ctx = &prop_ctx;
					printf("Switched to propositional mode\n\n");
					if (ans != NULL) tree_free(ans);
					ans = NULL;
					continue;
				}
				if (strcmp(input, "help") == 0)
				{
					print_ops(curr_ctx);
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