#pragma once
#include "../tree/operator.h"
#include "../tree/node.h"

bool op_evaluate(const Operator *op, size_t num_args, const double *args, double *out);
double arith_evaluate(Node *node);
