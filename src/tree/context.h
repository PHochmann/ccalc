#pragma once
#include <stdbool.h>
#include "operator.h"

struct ParsingContext;

typedef struct ParsingContext
{
    size_t num_ops;      // Current count of operators 
    size_t max_ops;      // Maximum count of operators (limited by buffer size)
    Operator *glue_op;   // Points to op in operators
    Operator *operators; // Buffer of operators
} ParsingContext;

ParsingContext get_context(size_t max_ops, Operator *op_buffer);
bool ctx_add_ops(ParsingContext *ctx, size_t count, ...);
int ctx_add_op(ParsingContext *ctx, Operator op);
bool ctx_set_glue_op(ParsingContext *ctx, Operator *op);
void ctx_remove_glue_op(ParsingContext *ctx);
Operator *ctx_lookup_op(ParsingContext *ctx, char *name, OpPlacement placement);
Operator *ctx_lookup_function(ParsingContext *ctx, char *name, size_t arity);
bool ctx_is_function_overloaded(ParsingContext *ctx, char *name);
