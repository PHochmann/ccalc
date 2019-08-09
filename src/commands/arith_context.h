#pragma once
#include "../engine/context.h"
#include "../engine/node.h"

#define ARITH_NUM_OPS 49

double arith_eval(Node *node);
void arith_reset();
ParsingContext *arith_get_ctx();
