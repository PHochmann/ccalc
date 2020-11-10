#pragma once
#include "../../engine/tree/operator.h"
#include "../../engine/tree/node.h"

bool op_evaluate(const Operator *op, size_t num_args, const double *args, double *out);
double arith_evaluate(const Node *node);
