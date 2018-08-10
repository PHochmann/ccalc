#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "commands.h"
#include "arith.h"

#include "engine/constants.h"
#include "engine/node.h"
#include "engine/operator.h"
#include "engine/output.h"
#include "engine/tokenizer.h"
#include "engine/parser.h"
#include "engine/rule.h"
#include "engine/memory.h"
#include "engine/console_util.h"
#include "engine/str_util.h"

#define MAX_LINE_LENGTH 256
#define NUM_MAX_RULES 16

void parse_evaluation(char *input);
void print_help();
bool ask_input(char *out_input);
bool parse_node_wrapper(char *input, Node **out_res, bool apply_rules, bool apply_ans);
void add_function(char *input);
void add_rule(char *input);

bool debug; // When set to true, a tree and string representation is printed
int min_prio; // Minimal priority of messages that are printed

ParsingContext ctx; // ParsingContext to use while parsing strings
Node *ans; // Last parsed tree 'ans' is substituted with

RewriteRule rules[NUM_MAX_RULES];
int num_rules;

/*
Summary: Sets parsing context and prepares global vars
*/
void init_commands()
{
	ans = NULL;
	ctx = arith_get_ctx();
	num_rules = 0;
}

/*
Summary: Sets min_prio to 1, thus messages with priority of 0 are filtered
*/
void make_silent()
{
	min_prio = 1;
}

/*
Summary: Endless loop to ask user for input
*/
void main_interactive()
{
	char input[MAX_LINE_LENGTH];

	while (true)
	{
		if (ask_input(input))
		{
			parse_input(input);
		}
		else return;
	}
}

bool ask_input(char *out_input)
{
	printf("> ");
		
	if (fgets(out_input, MAX_LINE_LENGTH, stdin) != NULL)
	{
		out_input[strlen(out_input) - 1] = '\0';
		return true;
	}
	else
	{
		return false;
	}
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
		message(0, "toggled debug\n");
		return;
	}
	
	if (begins_with("def_rule ", input))
	{
		add_rule(input);
		return;
	}
	
	if (begins_with("def_func ", input))
	{
		add_function(input);
		return;
	}
	
	parse_evaluation(input);
}

void add_rule(char *input)
{
	if (num_rules == NUM_MAX_RULES)
	{
		printf("Maximum rules count exceeded\n");
		return;
	}
	
	input += 9;
	char left[MAX_LINE_LENGTH];
	int i = 0;
	
	while (!begins_with(" -> ", input) && input[0] != '\0')
	{
		left[i] = input[0];
		input++;
		i++;
	}
	left[i] = '\0';
	
	if (input[0] == '\0')
	{
		printf("Expected rule\n");
		return;
	}
	
	input += 4;
	
	Node *left_node;
	Node *right_node;
	
	if (!parse_node_wrapper(left, &left_node, false, true)) return;
	if (!parse_node_wrapper(input, &right_node, false, true)) return;
	
	rules[num_rules].before = left_node;
	rules[num_rules].after = right_node;
	rules[num_rules].context = &ctx;
	num_rules++;
}

void add_function(char *input)
{
	input += 9;
	int i = 0;
	
	char *name = malloc(sizeof(char) * MAX_OP_LENGTH);
	while (input[0] == ' ' && input[0] != '\0') input++;
	while (is_letter(input[0]) && input[0] != '\0')
	{
		if (i == MAX_LINE_LENGTH)
		{
			printf("Max. operator length exceeded");
			return;
		}
		
		name[i++] = input[0];
		input++;
	}
	
	int arity = atoi(input);
	
	if (arity > MAX_CHILDREN)
	{
		printf("Max. arity is %d\n", MAX_CHILDREN);
	}
	
	int id = add_op(&ctx, op_get_function(name, arity));
	
	if (id == -1)
	{
		printf("Max. number of operators exceeded");
		return;
	}
}

void parse_evaluation(char *input)
{
	Node *res;
	
	if (parse_node_wrapper(input, &res, true, true))
	{
		if (debug)
		{
			show_tree(&ctx, res);
			printf("= ");
			inline_tree(&ctx, res);
			printf("\n");
		}
		
		char *vars[MAX_VAR_COUNT];
		int num_variables = list_variables(res, vars);
		for (int i = 0; i < num_variables; i++)
		{
			printf(" %s? ", vars[i]);
			char input[MAX_LINE_LENGTH];
			if (ask_input(input))
			{
				Node *res_var;
				if (!parse_node_wrapper(input, &res_var, true, true))
				{
					// Error while parsing - ask again
					i--;
					continue;
				}
				
				if (tree_contains_variable(res_var))
				{
					// Not a constant given - ask again
					printf("Not a constant expression\n");
					i--;
					continue;
				}
				
				tree_substitute(&ctx, res, res_var, vars[i]);
			}
			else return;
		}
		
		double eval = arith_eval(res);
		char result_str[ctx.min_strbuf_length];
		ctx.to_string((void*)(&eval), result_str, ctx.min_strbuf_length);
		printf("= %s\n", result_str);
		
		if (ans != NULL) free_tree(ans);
		ans = res;
	}
}

bool parse_node_wrapper(char *input, Node **out_res, bool apply_rules, bool apply_ans)
{
	ParserError perr = parse_node(&ctx, input, out_res);
	if (perr != PERR_SUCCESS)
	{
		printf("Error: %s\n", perr_to_string(perr));
		return false;
	}
	
	if (apply_ans && ans != NULL) tree_substitute(&ctx, *out_res, ans, "ans");
	if (apply_rules)
	{
		int applied_rules = apply_ruleset(*out_res, rules, num_rules, 100);
		if (applied_rules != 0) message(0, "%d rules applied\n", applied_rules);
	}
	
	return true;
}

void print_help()
{
	for (int i = 0; i < ctx.num_ops; i++)
	{
		printf(OP_COLOR "%s" COL_RESET " ", ctx.operators[i].name);
	}
	printf("\n(%d available operators)\n", ctx.num_ops);
}

/*
Summary: printf-wrapper to filter unimportant prints in silent mode
*/
void message(int prio, const char *format, ...)
{
	va_list args;
    va_start(args, format);

    if(prio >= min_prio) vprintf(format, args);

    va_end(args);
}
