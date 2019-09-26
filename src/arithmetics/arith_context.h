#pragma once
#include "../parsing/context.h"
#include "../parsing/node.h"

// Exported for help command
extern const size_t ARITH_NUM_OPS;

double arith_eval(Node *node);
void arith_reset_ctx();
ParsingContext *arith_init_ctx();
