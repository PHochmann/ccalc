#pragma once
#include "../parsing/context.h"
#include "../parsing/node.h"

#define ARITH_NUM_OPS 52
#define ARITH_MAX_OPS (ARITH_NUM_OPS + 10)
#define g_ctx (&__g_ctx)

extern ParsingContext __g_ctx;

double arith_eval(Node *node);
double op_eval(Operator *op, size_t num_children, double *children);
void arith_reset_ctx();
void arith_unload_ctx();
void arith_init_ctx();
