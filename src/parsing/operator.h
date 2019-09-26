#pragma once
#include <stdlib.h>

typedef enum
{
    OP_ASSOC_RIGHT,
    OP_ASSOC_LEFT,
} OpAssociativity;

typedef enum
{
    OP_PLACE_PREFIX,
    OP_PLACE_INFIX,
    OP_PLACE_POSTFIX,
    OP_PLACE_FUNCTION,
} OpPlacement;

typedef unsigned char Precedence;

extern const size_t DYNAMIC_ARITY;
extern const size_t MAX_ARITY;
extern const Precedence MAX_PRECEDENCE;
extern const OpAssociativity STANDARD_ASSOC;

typedef struct
{
    char *name; // On heap
    size_t arity;
    Precedence precedence;
    OpAssociativity assoc;
    OpPlacement placement;
} Operator;

Operator op_get_function(char *name, size_t arity);
Operator op_get_prefix(char *name, Precedence precedence);
Operator op_get_infix(char *name, Precedence precedence, OpAssociativity assoc);
Operator op_get_postfix(char *name, Precedence precedence);
Operator op_get_constant(char *name);
