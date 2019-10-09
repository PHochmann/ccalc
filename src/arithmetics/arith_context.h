#pragma once
#include "../parsing/context.h"
#include "../parsing/node.h"

#define g_ctx (&__g_ctx)

#define ARITH_MAX_OPS 60
#define ARITH_NUM_OPS 50

extern ParsingContext __g_ctx;

double arith_eval(Node *node);
void arith_reset_ctx();
void arith_unload_ctx();
void arith_init_ctx();
