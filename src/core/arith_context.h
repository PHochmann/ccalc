#pragma once
#include <stdbool.h>
#include "../parsing/context.h"
#include "../tree/node.h"
#include "../transformation/rewrite_rule.h"

#define g_ctx (&__g_ctx)

extern ParsingContext __g_ctx;

void init_core_ctx();
size_t get_num_composite_functions();
RewriteRule *get_composite_function(size_t index);
bool can_add_composite_function();
void add_composite_function(RewriteRule rule);
void pop_composite_function();
void clear_composite_functions();
bool arith_parse_input(char *input, char *error_fmt, Node **out_res);
bool arith_parse_input_extended(char *input, char *error_fmt, bool replace_comp_funcs, bool simplify, bool debug, Node **out_res);
