#include <stdlib.h>
#include <stdbool.h>

#include <stdio.h>

#include "context.h"

/*
Summary: This method is used to create a new ParsingContext. You may want to set a glueOp afterwards.
Parameters
	val_size: Size of a constant in bytes (e.g. sizeof(double) for arithmetics, sizeof(bool) for propositional logic)
	min_strbuf_length: Minimal buffer size to be supplied to to_string when printing a constant (not relevant for engine, just for own account)
	max_ops: Number of operators that should fit into reserved buffer
	try_parse: Function that is called when trying to parse a constant
	to_string: Function that makes a constant readable
	equals: Function that compares to constants (when NULL is given, bytewise_equals is used as a fallback) (only relevant for matching in rule.c)
*/
ParsingContext get_context(size_t val_size,
	size_t min_strbuf_length,
	int max_ops,
	TryParseHandler try_parse,
	ToStringHandler to_string,
	EqualsHandler equals)
{
	ParsingContext res = (ParsingContext){
		.value_size = val_size,
		.min_strbuf_length = min_strbuf_length,
		.max_ops = max_ops,
		.num_ops = 0,
		.try_parse = try_parse,
		.to_string = to_string,
		.equals = equals,
		.operators = malloc(sizeof(Operator) * max_ops),
		.glue_op = NULL,
	};

	return res;
}

/*
Summary: Adds operator to context
Returns: ID of new operator, -1 if buffer is full or infix operator with inconsistent associativity is given
*/
int add_op(ParsingContext *ctx, Operator op)
{
	if (ctx == NULL) return -1;
	if (ctx->num_ops == ctx->max_ops) return -1; // Buffer too small
	
	// For infix operators: Operators with same precedence must have the same associativity
	// to associate every operand with exactly one operator in a unique manner
	if (op.placement == OP_PLACE_INFIX)
	{
		for (int i = 0; i < ctx->num_ops; i++)
		{
			if (ctx->operators[i].placement == OP_PLACE_INFIX && ctx->operators[i].precedence == op.precedence)
			{
				// The parser treats OP_ASSOC_BOTH as OP_ASSOC_LEFT
				OpAssociativity a_assoc = ctx->operators[i].assoc;
				OpAssociativity b_assoc = op.assoc;
				
				if (a_assoc == OP_ASSOC_BOTH) a_assoc = OP_ASSOC_LEFT;
				if (b_assoc == OP_ASSOC_BOTH) b_assoc = OP_ASSOC_LEFT;
				
				if (a_assoc != b_assoc)
				{
					return -1;
				}
			}
		}
	}
	
	ctx->operators[ctx->num_ops++] = op;
	
	return ctx->num_ops - 1;
}

bool set_glue_op(ParsingContext *ctx, Operator *op)
{
	if (ctx == NULL) return false;
	if (op->placement != OP_PLACE_INFIX) return false; // Must be infix to "glue" subtrees together
	
	ctx->glue_op = op;
	return true;
}

void remove_glue_op(ParsingContext *ctx)
{
	if (ctx == NULL) return;
	
	ctx->glue_op = NULL;
}
