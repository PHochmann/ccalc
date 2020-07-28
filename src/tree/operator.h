#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

#define OP_DYNAMIC_ARITY  SIZE_MAX  // Used to indicate arbitrary number of operands
#define OP_MAX_PRECEDENCE UCHAR_MAX // Since precedence is an unsigned char

typedef unsigned char Precedence;

typedef enum {
    OP_ASSOC_RIGHT,
    OP_ASSOC_LEFT,
} OpAssociativity;

typedef enum {
    OP_PLACE_PREFIX,
    OP_PLACE_INFIX,
    OP_PLACE_POSTFIX,
    OP_PLACE_FUNCTION,
} OpPlacement;

typedef struct {
    char *name;
    size_t id;
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
