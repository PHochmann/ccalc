#pragma once
#include "../engine/context.h"
#include "../engine/node.h"

// In header file so that help command detects user-defined functions in ctx
extern const size_t ARITH_NUM_OPS;

double arith_eval(Node *node);
void arith_reset();
ParsingContext *arith_get_ctx();
