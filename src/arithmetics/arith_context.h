#pragma once
#include "../tree/context.h"
#include "../tree/node.h"

#define ARITH_NUM_OPS 54
#define ARITH_MAX_OPS (ARITH_NUM_OPS + 10)
#define g_ctx (&__g_ctx)

extern ParsingContext __g_ctx;

double arith_eval(Node *node);
double op_eval(Operator *op, size_t num_children, double *children);
void arith_init_ctx();
void arith_reset_ctx();
