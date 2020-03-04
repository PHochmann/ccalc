#pragma once
#include "../tree/context.h"
#include "../tree/node.h"
#include "../transformation/rewrite_rule.h"

#define g_ctx (&__g_ctx)

extern ParsingContext __g_ctx;

void core_init_ctx();
size_t get_num_composite_functions();
RewriteRule *get_composite_function(size_t index);
bool can_add_composite_function();
void add_composite_function(RewriteRule rule);
void pop_composite_function();
void clear_composite_functions();
bool core_parse_input(char *input, char *error_fmt, bool replace_comp_funcs, Node **out_res);
