#pragma once
#include <stdlib.h>

// Used to indicate arbitrary number of operands (0 up to MAX_ARITY)
// Arities are encoded as size_t, so it could be much higher
#define OP_DYNAMIC_ARITY 101
// One less than DYNAMIC_ARITY
#define OP_MAX_ARITY 100
// Since precendece is a char
#define OP_MAX_PRECEDENCE 255

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
