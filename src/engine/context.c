#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "context.h"

/*
Summary: This method is used to create a new ParsingContext without glue-op
Parameters:
    value_size: Size of a constant in bytes
        (e.g. sizeof(double) for arithmetics, sizeof(bool) for propositional logic)
    min_str_len: Minimum amount of chars (without \0) a buffer supplied to to_string should hold
        (not relevant for engine, only for own account)
    max_ops: Number of operators that should fit into reserved buffer
    try_parse: Function that is called when trying to parse a constant
    to_string: Function that makes a constant readable
    equals: Function that compares two constants. When NULL is given, bytewise_equals is used as a fallback.   
        (Only relevant for node_equals and tree_equals used in rule.c)
*/
ParsingContext get_context(
    size_t value_size,
    size_t min_str_len,
    size_t max_ops,
    TryParseHandler try_parse,
    ToStringHandler to_string,
    EqualsHandler equals)
{
    ParsingContext res = (ParsingContext){
        .value_size = value_size,
        .min_str_len = min_str_len,
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
Summary: Adds given operators to context
Returns: true if all operators were successfully added, false if inconsistency occured, buffer full or invalid arguments
*/
bool ctx_add_ops(ParsingContext *ctx, int count, ...)
{
    va_list args;
    va_start(args, count);
    
    for (int i = 0; i < count; i++)
    {
        if (ctx_add_op(ctx, va_arg(args, Operator)) == -1)
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
Returns: ID of new operator, -1 if buffer is full or infix operator with inconsistent associativity is given
*/
int ctx_add_op(ParsingContext *ctx, Operator op)
{
    if (ctx == NULL || ctx->num_ops == ctx->max_ops) return -1;
    
    // Consistency checks:
    
    // Check if operator with same name already exists
    // For functions: Check if function with same name and arity already exists, since function overloading is supported
    if (op.placement != OP_PLACE_FUNCTION)
    {
        if (ctx_lookup_op(ctx, op.name, op.placement) != NULL) return -1;
    }
    else
    {
        if (ctx_lookup_function(ctx, op.name, op.arity) != NULL) return -1;
    }
    
    // For infix operators: Operators with same precedence must have the same associativity
    // to associate every operand with exactly one operator in a unique manner
    if (op.placement == OP_PLACE_INFIX)
    {
        for (int i = 0; i < ctx->num_ops; i++)
        {
            if (ctx->operators[i].placement == OP_PLACE_INFIX
                && ctx->operators[i].precedence == op.precedence)
            {
                OpAssociativity a_assoc = ctx->operators[i].assoc;
                OpAssociativity b_assoc = op.assoc;
                
                // Parser treats OP_ASSOC_BOTH as OP_ASSOC_LEFT
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

/*
Summary: Sets glue-op, which is inserted between two subexpressions (such as 2a -> 2*a)
Returns: False, if null in arguments or operator with arity other than 2 given
*/
bool ctx_set_glue_op(ParsingContext *ctx, Operator *op)
{
    if (ctx == NULL || op == NULL || op->arity != 2) return false;
    ctx->glue_op = op;
    return true;
}

/*
Summary: Sets glue-op to NULL, two subexpressions next to each other will result in PERR_UNEXPECTED_SUBEXPRESSION
*/
void remove_glue_op(ParsingContext *ctx)
{
    if (ctx == NULL) return;
    ctx->glue_op = NULL;
}

/*
Summmary: Searches for operator of given name and placement
Returns: NULL if no operator has been found or invalid arguments given, otherwise pointer to operator in ctx->operators
*/
Operator* ctx_lookup_op(ParsingContext *ctx, char *name, OpPlacement placement)
{
    if (ctx == NULL || name == NULL) return NULL;

    for (int i = 0; i < ctx->num_ops; i++)
    {
        Operator *curr_op = &(ctx->operators[i]);
        
        if (curr_op->placement == placement
            && strcmp(curr_op->name, name) == 0)
        {
            return curr_op;
        }
    }
    
    return NULL;
}

/*
Summmary: Searches for function of given name and arity
Returns: NULL if no function has been found or invalid arguments given, otherwise pointer to function in ctx->operators
*/
Operator* ctx_lookup_function(ParsingContext *ctx, char *name, int arity)
{
    if (ctx == NULL || name == NULL) return NULL;

    for (int i = 0; i < ctx->num_ops; i++)
    {
        Operator *curr_op = &ctx->operators[i];
        
        if (curr_op->placement == OP_PLACE_FUNCTION
            && strcmp(curr_op->name, name) == 0
            && curr_op->arity == arity)
        {
            return curr_op;
        }
    }
    
    return NULL;
}
