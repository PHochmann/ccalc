#include <limits.h>

#include "operator.h"

Operator op_get_function(char *name, unsigned int arity)
{
    Operator res;
    
    res.name = name;
    res.arity = arity;
    
    res.precedence = UINT_MAX;
    res.placement = OP_PLACE_FUNCTION;
    
    return res;
}

Operator op_get_prefix(char *name, unsigned int precedence)
{
    Operator res;
    
    res.name = name;
    res.arity = 1;
    
    res.precedence = precedence;
    res.placement = OP_PLACE_PREFIX;
    res.assoc = OP_ASSOC_LEFT;
    
    return res;
}

Operator op_get_infix(char *name, unsigned int precedence, OpAssociativity assoc)
{
    Operator res;
    
    res.name = name;
    res.arity = 2;
    
    res.precedence = precedence;
    res.assoc = assoc;
    res.placement = OP_PLACE_INFIX;
    
    return res;
}

Operator op_get_postfix(char *name, unsigned int precedence)
{
    Operator res;
    
    res.name = name;
    res.arity = 1;
    
    res.precedence = precedence;
    res.placement = OP_PLACE_POSTFIX;
    res.assoc = OP_ASSOC_LEFT; // To pop prefix operators before postfix operators of same arity
    
    return res;
}

Operator op_get_constant(char *name)
{
    Operator res;
    
    res.name = name;
    res.arity = 0;
    
    res.precedence = UINT_MAX;
    res.placement = OP_PLACE_PREFIX; // Needed for await_subexpression = false
    
    return res;
}
