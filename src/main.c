#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "engine/constants.h"
#include "engine/node.h"
#include "engine/operator.h"
#include "engine/output.h"
#include "engine/tokenizer.h"
#include "engine/parser.h"
#include "engine/rule.h"
#include "engine/memory.h"
#include "engine/console_util.h"

#include "arithmetic/arith.h"

#define VERSION "0.0.5"
#define MAX_LINE_LENGTH 256

bool debug = false;
bool silent = false;
ParsingContext ctx;
Node *ans;

void _exit()
{
	uninit_parser();
}

void print_help()
{
	for (int i = 0; i < ctx.num_ops; i++)
	{
		printf(OP_COLOR "%s" COL_RESET " ", ctx.operators[i].name);
	}
	printf("\n(%d available operators)\n", ctx.num_ops);
}

void parse_input(char *input)
{
	if (strcmp(input, "exit") == 0) exit(0);
	if (strcmp(input, "help") == 0)
	{
		print_help();
		return;
	}
	if (strcmp(input, "debug") == 0)
	{
		debug = !debug;
		if (!silent) printf("toggled debug\n");
		return;
	}
	
	Node *res;
	ParserError perr = parse_node(&ctx, input, &res);
	
	if (perr == PERR_SUCCESS)
	{
		if (ans != NULL) tree_substitute(&ctx, res, ans, "ans");
		
		if (debug)
		{
			show_tree(&ctx, res);
			printf("= ");
			inline_tree(&ctx, res);
			printf("\n");
		}
		
		// Evaluate result if tree is constant
		if (!tree_contains_variable(res))
		{
			double eval = arith_eval(res);
			char result_str[ctx.min_strbuf_length];
			ctx.to_string((void*)(&eval), result_str, ctx.min_strbuf_length);
			printf("= %s\n", result_str);
		}
		if (debug) printf("\n");
		
		if (ans != NULL) free_tree(ans);
		ans = res;
	}
	else
	{
		printf("Error: %s\n", perr_to_string(perr));
	}
}

int main(int argc, char *argv[])
{
	atexit(_exit);

	ctx = arith_get_ctx();
	ans = NULL;
	init_parser();
	
	if (argc > 1)
	{
		silent = true;
		for (int i = 1; i < argc; i++) parse_input(argv[i]);
		if (!debug) printf("\n");
	}
	else
	{
		printf("Calculator %s (c) 2018, Philipp Hochmann\n", VERSION);
		char input[MAX_LINE_LENGTH];
		
		while (true)
		{
			printf("\n> ");
			
			if (fgets(input, MAX_LINE_LENGTH, stdin) != NULL)
			{
				input[strlen(input) - 1] = '\0';
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
