#pragma once
#include <stdbool.h>
#include "../../engine/tree/operator.h"
#include "../../engine/tree/node.h"
#include "../../engine/tree/tree_util.h"

ListenerError prop_op_evaluate(const Operator *op, size_t num_args, const double *args, double *out);
bool propositional_checker(Node **tree);
