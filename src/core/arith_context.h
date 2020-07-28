#pragma once
#include <stdbool.h>
#include "../parsing/context.h"
#include "../tree/node.h"
#include "../transformation/rewrite_rule.h"

#define g_ctx (&__g_ctx)

extern ParsingContext __g_ctx;
extern LinkedList g_composite_functions;

void init_arith_ctx();
void unload_arith_ctx();

void add_composite_function(RewriteRule rule);
bool remove_composite_function(Operator *function);
void clear_composite_functions();
bool arith_parse_input_raw(char *input, char *error_fmt, Node **out_res);
bool arith_parse_input(char *input, char *error_fmt, bool replace_comp_funcs, Node **out_res);
