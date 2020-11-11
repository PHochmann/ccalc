#include "../core/arith_context.h"
#include "propositional_evaluation.h"

#define EVAL_TYPE_CONST 1
#define EVAL_TYPE_VAR   2
#define EVAL_TYPE_OP    4
#define EVAL_TRUE       1
#define EVAL_FALSE      0

bool prop_op_evaluate(const Operator *op, size_t num_args, const double *args, double *out)
{
    // Propositional context is an extension of the arithmetic context
    if (op->id < NUM_ARITH_OPS) return arith_op_evaluate(op, num_args, args, out);

    switch (op->id)
    {
        case NUM_ARITH_OPS + 1: // CONST
            *out = EVAL_TYPE_CONST;
            return true;
        case NUM_ARITH_OPS + 2: // VAR
            *out = EVAL_TYPE_VAR;
            return true;
        case NUM_ARITH_OPS + 3: // OP
            *out = EVAL_TYPE_OP;
            return true;
        case NUM_ARITH_OPS + 4: // ==
            *out = (args[0] == args[1]) ? EVAL_TRUE : EVAL_FALSE;
            return true;
        case NUM_ARITH_OPS + 5: // >
            *out = (args[0] > args[1]) ? EVAL_TRUE : EVAL_FALSE;
            return true;
        case NUM_ARITH_OPS + 6: // <
            *out = (args[0] < args[1]) ? EVAL_TRUE : EVAL_FALSE;
            return true;
        case NUM_ARITH_OPS + 7: // >=
            *out = (args[0] >= args[1]) ? EVAL_TRUE : EVAL_FALSE;
            return true;
        case NUM_ARITH_OPS + 8: // <=
            *out = (args[0] <= args[1]) ? EVAL_TRUE : EVAL_FALSE;
            return true;
        case NUM_ARITH_OPS + 9: // OR
            *out = (args[0] == EVAL_TRUE || args[1] == EVAL_TRUE) ? EVAL_TRUE : EVAL_FALSE;
            return true;
        case NUM_ARITH_OPS + 10: // TRUE
            *out = EVAL_TRUE;
            return true;
        case NUM_ARITH_OPS + 11: // FALSE
            *out = EVAL_FALSE;
            return true;
    }

    software_defect("Software defect: [Prop] No reduction possible for operator %s.\n", op->name);
    return false;
}
