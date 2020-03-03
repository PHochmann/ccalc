#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "context.h"

/*
Summary: This method is used to create a new ParsingContext without glue-op and operators
    Use ctx_add_op and ctx_add_glue_op to add them to new context
Parameters:
    max_ops:   Number of operators that should fit into reserved buffer
    operators: Buffer to operators. Should hold max_ops operators.
*/
ParsingContext get_context(size_t max_ops, Operator *op_buffer)
{
    ParsingContext res = (ParsingContext){
        .max_ops = max_ops,
        .num_ops = 0,
        .operators = op_buffer,
        .glue_op = NULL,
    };

    return res;
}

/*
Summary: Adds given operators to context
Returns: true if all operators were successfully added
    false if inconsistency occurred, buffer full or invalid arguments
*/
bool ctx_add_ops(ParsingContext *ctx, size_t count, ...)
{
    va_list args;
    va_start(args, count);
    
    for (size_t i = 0; i < count; i++)
    {
        if (ctx_add_op(ctx, va_arg(args, Operator)) == NULL)
        {
            va_end(args);
            return false;
        }
    }
    
    va_end(args);
    return true;
}

/*
Summary: Adds operator to context
    To associate every operand with exactly one operator in a unique manner,
    infix operators with the same precedence must have the same associativity.
Returns: pointer to operator within context, NULL if one of the following:
    * buffer is full
    * ctx is NULL
    * infix operator with inconsistent associativity is given
          (another infix operator with same precedence has different associativity)
    * function of same name and arity is present in context
*/
Operator *ctx_add_op(ParsingContext *ctx, Operator op)
{
    if (ctx == NULL || ctx->num_ops == ctx->max_ops) return NULL;
    
    // Check for name clash
    if (op.placement != OP_PLACE_FUNCTION)
    {
        if (ctx_lookup_op(ctx, op.name, op.placement) != NULL) return NULL;
    }
    else
    {
        if (ctx_lookup_function(ctx, op.name, op.arity) != NULL) return NULL;
    }

    // Consistency checks
    if (op.placement == OP_PLACE_INFIX)
    {
        for (size_t i = 0; i < ctx->num_ops; i++)
        {
            if (ctx->operators[i].placement == OP_PLACE_INFIX
                && ctx->operators[i].precedence == op.precedence)
            {                
                if (ctx->operators[i].assoc != op.assoc)
                {
                    return NULL;
                }
            }
        }
    }

    ctx->operators[ctx->num_ops++] = op;
    return &ctx->operators[ctx->num_ops - 1];
}

/*
Summary: Sets glue-op, which is inserted between two subexpressions (such as 2a -> 2*a)
Returns: False, if ctx is NULL or operator with arity not equal to 2 or DYNAMIC_ARITY given
*/
bool ctx_set_glue_op(ParsingContext *ctx, Operator *op)
{
    if (ctx == NULL
        || (op != NULL
            && op->arity != 2
            && op->arity != OP_DYNAMIC_ARITY))
    {
        return false;
    }
    ctx->glue_op = op;
    return true;
}

// For function overloading: Returns function of given name. Favors zero-arity function when functions are overloaded.
Operator *lookup_tentative_function(ParsingContext *ctx, char *name)
{
    Operator *non_zero_func = NULL;

    for (size_t i = 0; i < ctx->num_ops; i++)
    {
        if (ctx->operators[i].placement == OP_PLACE_FUNCTION
            && strcmp(ctx->operators[i].name, name) == 0)
        {
            if (ctx->operators[i].arity == 0)
            {
                return &ctx->operators[i];
            }
            else
            {
                non_zero_func = &ctx->operators[i];
            }
        }
    }

    return non_zero_func;
}

/*
Summmary: Searches for operator of given name and placement
Returns: NULL if no operator has been found or invalid arguments given, otherwise pointer to operator in ctx->operators
*/
Operator *ctx_lookup_op(ParsingContext *ctx, char *name, OpPlacement placement)
{
    if (ctx == NULL || name == NULL) return NULL;
    if (placement == OP_PLACE_FUNCTION) return lookup_tentative_function(ctx, name);

    for (size_t i = 0; i < ctx->num_ops; i++)
    {
        if (ctx->operators[i].placement == placement
            && strcmp(ctx->operators[i].name, name) == 0)
        {
            return &ctx->operators[i];
        }
    }
    
    return NULL;
}

/*
Summmary: Searches for function of given name and arity
Returns: NULL if no function has been found or invalid arguments given, otherwise pointer to function in ctx->operators
*/
Operator *ctx_lookup_function(ParsingContext *ctx, char *name, size_t arity)
{
    if (ctx == NULL || name == NULL) return NULL;

    for (size_t i = 0; i < ctx->num_ops; i++)
    {
        if (ctx->operators[i].placement == OP_PLACE_FUNCTION
            && strcmp(ctx->operators[i].name, name) == 0
            && ctx->operators[i].arity == arity)
        {
            return &ctx->operators[i];
        }
    }
    
    return NULL;
}

/*
Returns: True if functions with same name but different arities exist, or arity of function is DYNAMIC_ARITY
*/
bool ctx_is_function_overloaded(ParsingContext *ctx, char *name)
{
    if (ctx == NULL || name == NULL) return false;

    bool found_first = false;
    for (size_t i = 0; i < ctx->num_ops; i++)
    {
        if (ctx->operators[i].placement == OP_PLACE_FUNCTION
            && strcmp(ctx->operators[i].name, name) == 0)
        {
            if (ctx->operators[i].arity == OP_DYNAMIC_ARITY) return true;
            if (found_first) return true;
            found_first = true;
        }
    }

    return false;
}
