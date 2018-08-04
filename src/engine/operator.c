#include <limits.h>

#include "operator.h"

Operator op_get_function(char *name, unsigned int arity)
{
	if (arity == 0) return op_get_constant(name);
	
	Operator res;
	
	res.name = name;
	res.arity = arity;
	
	res.precedence = UINT_MAX;
	res.assoc = OP_ASSOC_BOTH; // Not needed
	res.placement = OP_PLACE_FUNCTION;
	
	return res;
}

Operator op_get_prefix(char *name, unsigned int precedence)
{
	Operator res;
	
	res.name = name;
	res.arity = 1;
	
	res.precedence = precedence;
	res.assoc = OP_ASSOC_BOTH; // Not needed
	res.placement = OP_PLACE_PREFIX;
	
	return res;
}

Operator op_get_infix(char *name, unsigned int precedence, Op_Associativity assoc)
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
	res.assoc = OP_ASSOC_BOTH; // Not needed
	res.placement = OP_PLACE_POSTFIX;
	
	return res;
}

Operator op_get_constant(char *name)
{
	Operator res;
	
	res.name = name;
	res.arity = 0;
	
	res.precedence = UINT_MAX;
	res.assoc = OP_ASSOC_BOTH; // Not needed
	res.placement = OP_PLACE_PREFIX; // Needed for await_subexpression = false
	
	return res;
}
