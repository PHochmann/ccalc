#pragma once
#include <stdlib.h>
#include "constants.h"

typedef enum
{
    OP_ASSOC_RIGHT,
    OP_ASSOC_LEFT,
    OP_ASSOC_BOTH // Parser treats BOTH as STANDARD_ASSOC, only needed for printing
} OpAssociativity;

typedef enum
{
    OP_PLACE_PREFIX,
    OP_PLACE_INFIX,
    OP_PLACE_POSTFIX,
    OP_PLACE_FUNCTION,
} OpPlacement;

typedef unsigned char Arity;
typedef unsigned char Precedence;

typedef struct
{
    char *name;
    Arity arity;
    Precedence precedence;
    OpAssociativity assoc;
    OpPlacement placement;
} Operator;

Operator op_get_function(char *name, Arity arity);
Operator op_get_prefix(char *name, Precedence precedence);
Operator op_get_infix(char *name, Precedence precedence, OpAssociativity assoc);
Operator op_get_postfix(char *name, Precedence precedence);
Operator op_get_constant(char *name);
