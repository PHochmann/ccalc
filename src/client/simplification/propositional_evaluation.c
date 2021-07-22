#include "../../util/console_util.h"
#include "../../engine/tree/tree_util.h"

#include "../core/arith_context.h"
#include "../core/arith_evaluation.h"
#include "propositional_evaluation.h"
#include "propositional_context.h"

#define EVAL_TYPE_CONST 2
#define EVAL_TYPE_VAR   3
#define EVAL_TYPE_OP    4
#define EVAL_TRUE       1
#define EVAL_FALSE      0

ListenerError prop_op_evaluate(const Operator *op, size_t num_args, const double *args, double *out)
{
    // Propositional context is an extension of the arithmetic context
    if (op->id < NUM_ARITH_OPS) return arith_op_evaluate(op, num_args, args, out);

    switch (op->id)
    {
        case NUM_ARITH_OPS + 2: // ==
            *out = (args[0] == args[1]) ? EVAL_TRUE : EVAL_FALSE;
            return LISTENERERR_SUCCESS;
        case NUM_ARITH_OPS + 3: // !=
            *out = (args[0] != args[1]) ? EVAL_TRUE : EVAL_FALSE;
            return LISTENERERR_SUCCESS;
        case NUM_ARITH_OPS + 4: // CONST
            *out = EVAL_TYPE_CONST;
            return LISTENERERR_SUCCESS;
        case NUM_ARITH_OPS + 5: // VAR
            *out = EVAL_TYPE_VAR;
            return LISTENERERR_SUCCESS;
        case NUM_ARITH_OPS + 6: // OP
            *out = EVAL_TYPE_OP;
            return LISTENERERR_SUCCESS;
        case NUM_ARITH_OPS + 7: // >
            *out = (args[0] > args[1]) ? EVAL_TRUE : EVAL_FALSE;
            return LISTENERERR_SUCCESS;
        case NUM_ARITH_OPS + 8: // <
            *out = (args[0] < args[1]) ? EVAL_TRUE : EVAL_FALSE;
            return LISTENERERR_SUCCESS;
        case NUM_ARITH_OPS + 9: // >=
            *out = (args[0] >= args[1]) ? EVAL_TRUE : EVAL_FALSE;
            return LISTENERERR_SUCCESS;
        case NUM_ARITH_OPS + 10: // <=
            *out = (args[0] <= args[1]) ? EVAL_TRUE : EVAL_FALSE;
            return LISTENERERR_SUCCESS;
        case NUM_ARITH_OPS + 11: // ||
            *out = (args[0] != EVAL_FALSE || args[1] != EVAL_FALSE) ? EVAL_TRUE : EVAL_FALSE;
            return LISTENERERR_SUCCESS;
        case NUM_ARITH_OPS + 12: // TRUE
            *out = EVAL_TRUE;
            return LISTENERERR_SUCCESS;
        case NUM_ARITH_OPS + 13: // FALSE
            *out = EVAL_FALSE;
            return LISTENERERR_SUCCESS;
        case NUM_ARITH_OPS + 14: // !
            *out = (args[0] != EVAL_FALSE) ? EVAL_FALSE : EVAL_TRUE;
            return LISTENERERR_SUCCESS;
    }

    software_defect("[Prop] No reduction possible for operator %s.\n", op->name);
    return LISTENERERR_UNKNOWN_OP; // To make compiler happy
}

double equal_eval(__attribute__((unused)) size_t num_children, Node **children)
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

double type_eval(__attribute__((unused)) size_t num_children, Node **children)
{
    if (get_type(children[0]) == NTYPE_CONSTANT) return EVAL_TYPE_CONST;
    if (get_type(children[0]) == NTYPE_VARIABLE)
        //|| count_all_variable_nodes(*children) == 0)
    {
        return EVAL_TYPE_VAR;
    }
    if (get_type(children[0]) == NTYPE_OPERATOR) return EVAL_TYPE_OP;
    return 0; // To make compiler happy
}

bool propositional_checker(Node **tree)
{
    static const Operator *type_op = NULL;
    static const Operator *equal_op = NULL;
    
    if (type_op == NULL) type_op = ctx_lookup_op(g_propositional_ctx, "type", OP_PLACE_FUNCTION);
    if (equal_op == NULL) equal_op = ctx_lookup_op(g_propositional_ctx, "equal", OP_PLACE_FUNCTION);

    // Step 1: Reduce type(x)
    tree_reduce_ops(tree, type_op, type_eval);
    // Step 2: Reduce equal(x,y)
    tree_reduce_ops(tree, equal_op, equal_eval);
    // Step 3: Reduce everything else
    double reduced = 0;
    if (tree_reduce(*tree, prop_op_evaluate, &reduced, NULL) != LISTENERERR_SUCCESS)
    {
        return false;
    }

    return (reduced != EVAL_FALSE);
}
