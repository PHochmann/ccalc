#pragma once
#include <stdlib.h>

#include "constants.h"

typedef enum {
    OP_ASSOC_RIGHT,
    OP_ASSOC_LEFT,
    OP_ASSOC_BOTH // Parser treats BOTH as LEFT, only needed for printing
} OpAssociativity;

typedef enum {
    OP_PLACE_PREFIX,
    OP_PLACE_INFIX,
    OP_PLACE_POSTFIX,
    OP_PLACE_FUNCTION,
} OpPlacement;

typedef struct {
    
    char *name;
    unsigned int arity;
    unsigned int precedence;
    
    OpAssociativity assoc;
    OpPlacement placement;

} Operator;

Operator op_get_function(char *name, unsigned int arity);
Operator op_get_prefix(char *name, unsigned int precedence);
Operator op_get_infix(char *name, unsigned int precedence, OpAssociativity assoc);
Operator op_get_postfix(char *name, unsigned int precedence);
Operator op_get_constant(char *name);
