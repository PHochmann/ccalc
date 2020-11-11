#pragma once
#include <stdbool.h>
#include "../../engine/tree/operator.h"
#include "../../engine/tree/node.h"

bool prop_op_evaluate(const Operator *op, size_t num_args, const double *args, double *out);
