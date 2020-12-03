#pragma once
#include <stdbool.h>
#include "../../engine/parsing/context.h"
#include "../../engine/tree/node.h"
#include "../../engine/tree/tree_util.h"
#include "../../engine/transformation/rewrite_rule.h"

#define NUM_ARITH_OPS 57
#define g_ctx (&__g_ctx)
#define g_composite_functions (&__g_composite_functions)

extern ParsingContext __g_ctx;
extern LinkedList __g_composite_functions;

void init_arith_ctx();
ParsingContext get_arith_ctx();
void unload_arith_ctx();

void add_composite_function(RewriteRule rule);
bool remove_composite_function(const Operator *function);
void clear_composite_functions();
RewriteRule *get_composite_function(Operator *op);
bool arith_parse_raw(char *input, char *error_fmt, Node **out_res);
bool arith_parse_and_postprocess(char *input, char *error_fmt, Node **out_res);
ListenerError arith_postprocess(Node **tree);
const char *listenererr_to_str(ListenerError error);
