#pragma once
#include "node.h"
#include "context.h"

typedef enum {
    PERR_SUCCESS,                    // No error
    PERR_ARGS_MALFORMED,             // NULL-Pointer in arguments
    PERR_MAX_TOKENS_EXCEEDED,        // Token buffer overflow
    PERR_STACK_EXCEEDED,             // op-stack or node-stack overflow
    PERR_UNEXPECTED_SUBEXPRESSION,   // Only without glue-op: Two expressions next to each other
    PERR_EXCESS_OPENING_PARENTHESIS, // There are opening parenthesis on op-stack after all tokens are processed
    PERR_EXCESS_CLOSING_PARENTHESIS, // Mismatch: no opening parenthesis found
    PERR_UNEXPECTED_DELIMITER,       // Delimiter too soon (e.g. 1+,)
    PERR_MISSING_OPERATOR,           // Too many children
    PERR_MISSING_OPERAND,            // Could not pop 'arity' amount of nodes
    PERR_OUT_OF_MEMORY,              // Malloc failed
    PERR_FUNCTION_WRONG_ARITY,       // Function of wrong arity
    PERR_CHILDREN_EXCEEDED,          // Too many operands for function
    PERR_EMPTY,                      // Not a single node
} ParserError;

bool try_parse_constant(char *in, ConstantType *out);
ParserError parse_tokens(ParsingContext *ctx, int num_tokens, char **tokens, Node **out_res);
ParserError parse_input(ParsingContext *ctx, char *input, Node **out_res);
Node *parse_conveniently(ParsingContext *ctx, char *input);
