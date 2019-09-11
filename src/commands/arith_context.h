#pragma once
#include "../engine/context.h"
#include "../engine/node.h"

// Exported to let help command detect user-defined functions in ctx
extern const size_t ARITH_NUM_OPS;

double arith_eval(Node *node);
void arith_reset_ctx();
ParsingContext *arith_init_ctx();
