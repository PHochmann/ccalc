#include "../../engine/util/console_util.h"
#include "../core/arith_context.h"
#include "propositional_context.h"

#define NUM_PROPOSITIONAL_OPS 13

ParsingContext __g_propositional_ctx;

void init_propositional_ctx()
{
    __g_propositional_ctx = get_arith_ctx();
    if (!ctx_add_ops(g_propositional_ctx, NUM_PROPOSITIONAL_OPS,
        op_get_function("type", 1),
        op_get_infix("=", 1, OP_ASSOC_LEFT),
        op_get_constant("CONST"),
        op_get_constant("VAR"),
        op_get_constant("OP"),
        op_get_infix("==", 1, OP_ASSOC_LEFT),
        op_get_infix(">", 3, OP_ASSOC_LEFT),
        op_get_infix("<", 3, OP_ASSOC_LEFT),
        op_get_infix(">=", 3, OP_ASSOC_LEFT),
        op_get_infix("<=", 3, OP_ASSOC_LEFT),
        op_get_infix("OR", 2, OP_ASSOC_LEFT),
        op_get_constant("TRUE"),
        op_get_constant("FALSE")))
    {
        software_defect("[Prop] Inconsistent operator set.\n");
    }
}

void unload_propositional_ctx()
{
    ctx_destroy(g_propositional_ctx);
}
