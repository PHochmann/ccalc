#pragma once
#include "../tree/node.h"

bool op_evaluate(Operator *op, size_t num_args, double *args, double *out);
double arith_evaluate(Node *node);
