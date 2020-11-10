#include "../core/arith_context.h"
#include "extended_context.h"

#define NUM_EXTENSION_OPS 10

void init_extended_ctx()
{
    __g_extended_ctx = get_arith_ctx();
    ctx_add_ops(g_extended_ctx, NUM_EXTENSION_OPS,
        op_get_function("type", 1),
        op_get_constant("CONST"),
        op_get_constant("VAR"),
        op_get_constant("OP"),
        op_get_infix("=", 1, OP_ASSOC_LEFT),
        op_get_infix(">", 3, OP_ASSOC_LEFT),
        op_get_infix("<", 3, OP_ASSOC_LEFT),
        op_get_infix("OR", 2, OP_ASSOC_LEFT),
        op_get_constant("TRUE"),
        op_get_constant("FALSE"));
}

void unload_extended_ctx()
{
    ctx_destroy(g_extended_ctx);
}
