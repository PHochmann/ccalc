#pragma once
#include "../tree/node.h"
#include "context.h"

typedef enum {
    PERR_SUCCESS,                    // No error
    PERR_ARGS_MALFORMED,             // NULL-pointer in arguments
    PERR_UNEXPECTED_SUBEXPRESSION,   // Only without glue-op: Two expressions next to each other
    PERR_EXCESS_OPENING_PARENTHESIS, // There are opening parenthesis on op-stack after all tokens are processed
    PERR_EXCESS_CLOSING_PARENTHESIS, // Mismatch: no opening parenthesis found
    PERR_UNEXPECTED_DELIMITER,       // Delimiter too soon (e.g. 1+,) or not within parameter list of a function
    PERR_MISSING_OPERATOR,           // Too many children
    PERR_MISSING_OPERAND,            // Could not pop 'arity' amount of nodes
    PERR_FUNCTION_WRONG_ARITY,       // Function of wrong arity
    PERR_CHILDREN_EXCEEDED,          // Too many operands for function
    PERR_EMPTY,                      // No node has been created
} ParserError;

bool try_parse_constant(const char *in, ConstantType *out);
ParserError parse_tokens(const ParsingContext *ctx, int num_tokens, const char **tokens, Node **out_res);
ParserError parse_input(const ParsingContext *ctx, const char *input, Node **out_res);
Node *parse_conveniently(const ParsingContext *ctx, const char *input);
