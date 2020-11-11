#include "../../engine/util/console_util.h"
#include "../../engine/tree/tree_util.h"

#include "../core/arith_context.h"
#include "../core/arith_evaluation.h"
#include "propositional_evaluation.h"
#include "propositional_context.h"

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
        case NUM_ARITH_OPS + 2: // CONST
            *out = EVAL_TYPE_CONST;
            return true;
        case NUM_ARITH_OPS + 3: // VAR
            *out = EVAL_TYPE_VAR;
            return true;
        case NUM_ARITH_OPS + 4: // OP
            *out = EVAL_TYPE_OP;
            return true;
        case NUM_ARITH_OPS + 5: // ==
            *out = (args[0] == args[1]) ? EVAL_TRUE : EVAL_FALSE;
            return true;
        case NUM_ARITH_OPS + 6: // >
            *out = (args[0] > args[1]) ? EVAL_TRUE : EVAL_FALSE;
            return true;
        case NUM_ARITH_OPS + 7: // <
            *out = (args[0] < args[1]) ? EVAL_TRUE : EVAL_FALSE;
            return true;
        case NUM_ARITH_OPS + 8: // >=
            *out = (args[0] >= args[1]) ? EVAL_TRUE : EVAL_FALSE;
            return true;
        case NUM_ARITH_OPS + 9: // <=
            *out = (args[0] <= args[1]) ? EVAL_TRUE : EVAL_FALSE;
            return true;
        case NUM_ARITH_OPS + 10: // OR
            *out = (args[0] == EVAL_TRUE || args[1] == EVAL_TRUE) ? EVAL_TRUE : EVAL_FALSE;
            return true;
        case NUM_ARITH_OPS + 11: // TRUE
            *out = EVAL_TRUE;
            return true;
        case NUM_ARITH_OPS + 12: // FALSE
            *out = EVAL_FALSE;
            return true;
    }

    software_defect("Software defect: [Prop] No reduction possible for operator %s.\n", op->name);
    return false;
}

double type_eval(__attribute__((unused)) size_t num_children, Node **children)
{
    if (tree_equals(children[0], children[1]))
    {
        return EVAL_TRUE;
    }
    else
    {
        return EVAL_FALSE;
    }
}

double equals_eval(__attribute__((unused)) size_t num_children, Node **children)
{
    if (get_type(children[0]) == NTYPE_CONSTANT) return EVAL_TYPE_CONST;
    if (get_type(children[0]) == NTYPE_OPERATOR) return EVAL_TYPE_OP;
    if (get_type(children[0]) == NTYPE_VARIABLE) return EVAL_TYPE_VAR;
    return 0; // To make compiler happy
}

bool propositional_checker(Node **tree)
{
    // Step 1: Reduce type(x)
    tree_reduce_ops(tree, ctx_lookup_op(g_propositional_ctx, "type", OP_PLACE_FUNCTION), type_eval);

    // Step 2: Reduce =
    tree_reduce_ops(tree, ctx_lookup_op(g_propositional_ctx, "=", OP_PLACE_INFIX), equals_eval);

    // Step 3: Reduce everything else
    double reduced = 0;
    if (!tree_reduce(*tree, prop_op_evaluate, &reduced))
    {
        return false;
    }
    return (reduced == EVAL_TRUE);
}
