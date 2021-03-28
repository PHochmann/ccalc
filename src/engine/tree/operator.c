#include "operator.h"

Operator op_get_function(const char *name, size_t arity)
{
    return (Operator){
        .name       = (char*)name,
        .arity      = arity,
        .precedence = OP_MAX_PRECEDENCE,
        .placement  = OP_PLACE_FUNCTION,
        .assoc      = OP_ASSOC_LEFT
    };
}

Operator op_get_prefix(const char *name, Precedence precedence)
{
    return (Operator){
        .name       = (char*)name,
        .arity      = 1,
        .precedence = precedence,
        .placement  = OP_PLACE_PREFIX,
        .assoc      = OP_ASSOC_LEFT
    };
}

Operator op_get_infix(const char *name, Precedence precedence, OpAssociativity assoc)
{
    return (Operator){
        .name       = (char*)name,
        .arity      = 2,
        .precedence = precedence,
        .assoc      = assoc,
        .placement  = OP_PLACE_INFIX
    };
}

Operator op_get_postfix(const char *name, Precedence precedence)
{
    return (Operator){
        .name       = (char*)name,
        .arity      = 1,
        .precedence = precedence,
        .placement  = OP_PLACE_POSTFIX,
        .assoc      = OP_ASSOC_LEFT
    };
}

// Constants are just zero-arity functions
Operator op_get_constant(const char *name)
{
    return op_get_function(name, 0);
}
