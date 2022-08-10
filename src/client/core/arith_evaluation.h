#pragma once
#include <stdbool.h>
#include "../../engine/tree/operator.h"
#include "../../engine/tree/node.h"
#include "../../engine/tree/tree_util.h"

#define LISTENERERR_HISTORY_NOT_SET   1
#define LISTENERERR_IMPOSSIBLE_DERIV  2
#define LISTENERERR_MALFORMED_DERIV_A 3
#define LISTENERERR_MALFORMED_DERIV_B 4
#define LISTENERERR_UNKNOWN_OP        5
#define LISTENERERR_DIVISION_BY_ZERO  6
#define LISTENERERR_COMPLEX_SOLUTION  7
#define LISTENERERR_EMPTY_PARAMS      8

ListenerError arith_op_evaluate(const Operator *op, size_t num_args, const double *args, double *out);
double arith_evaluate(const Node *node);
