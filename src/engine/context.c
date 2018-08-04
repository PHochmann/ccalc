#include <stdlib.h>
#include <stdbool.h>

#include <stdio.h>

#include "context.h"

ParsingContext get_context(size_t val_size, size_t min_strbuf_length, int max_ops, TryParseHandler try_parse, ToStringHandler to_string)
{
	ParsingContext res = (ParsingContext){
		.value_size = val_size,
		.min_strbuf_length = min_strbuf_length,
		.max_ops = max_ops,
		.num_ops = 0,
		.try_parse = try_parse,
		.to_string = to_string,
		.operators = malloc(sizeof(Operator) * max_ops),
		.glue_op = NULL,
	};

	return res;
}

/*
Summary: adds operator to context
Returns: ID of new operator, -1 if buffer is full
*/
int add_op(ParsingContext *ctx, Operator op)
{
	if (ctx == NULL) return -1;
	
	if (ctx->num_ops == ctx->max_ops) return -1; // Buffer too small
	
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
