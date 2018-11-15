#include <limits.h>

#include "operator.h"

Operator op_get_function(char *name, unsigned int arity)
{
    return (Operator){
        .name = name,
        .arity = arity,
        .precedence = UINT_MAX,
        .placement = OP_PLACE_FUNCTION,
        .assoc = OP_ASSOC_LEFT
    };
}

Operator op_get_prefix(char *name, unsigned int precedence)
{
    return (Operator){
        .name = name,
        .arity = 1,
        .precedence = precedence,
        .placement = OP_PLACE_PREFIX,
        .assoc = OP_ASSOC_LEFT
    };
}

Operator op_get_infix(char *name, unsigned int precedence, OpAssociativity assoc)
{
    return (Operator){
        .name = name,
        .arity = 2,
        .precedence = precedence,
        .assoc = assoc,
        .placement = OP_PLACE_INFIX
    };
}

Operator op_get_postfix(char *name, unsigned int precedence)
{
    return (Operator){
        .name = name,
        .arity = 1,
        .precedence = precedence,
        .placement = OP_PLACE_POSTFIX,
        .assoc = OP_ASSOC_LEFT // To pop prefix operators before postfix operators of same arity
    };
}

Operator op_get_constant(char *name)
{
    return (Operator){
        .name = name,
        .arity = 0,
        .precedence = UINT_MAX,
        .placement = OP_PLACE_PREFIX, // Needed for await_subexpression = false
        .assoc = OP_ASSOC_RIGHT
    };
}
