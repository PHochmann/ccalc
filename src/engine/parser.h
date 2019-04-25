#pragma once
#include "node.h"
#include "context.h"

typedef enum {
    // No error
    PERR_SUCCESS,
    // NULL-Pointer in arguments
    PERR_ARGS_MALFORMED,
    // Token buffer overflow
    PERR_MAX_TOKENS_EXCEEDED,
    // op-stack or node-stack overflow
    PERR_STACK_EXCEEDED,
    // Only without glue-op: Two expressions next to each other
    PERR_UNEXPECTED_SUBEXPRESSION,
    // There are opening parenthesis on op-stack after all tokens are processed
    PERR_EXCESS_OPENING_PARENTHESIS,
    // Mismatch: no opening parenthesis found
    PERR_EXCESS_CLOSING_PARENTHESIS,
    // Delimiter too soon (e.g. 1+,)
    PERR_UNEXPECTED_DELIMITER,
    // Too many children
    PERR_MISSING_OPERATOR,
    // Could not pop 'arity' amount of nodes
    PERR_MISSING_OPERAND,
    // Malloc failed
    PERR_OUT_OF_MEMORY,
    // Function of wrong arity
    PERR_FUNCTION_WRONG_ARITY,
    // Too many operands for function
    PERR_CHILDREN_EXCEEDED,
    // Not a single node
    PERR_EMPTY,
} ParserError;

ParserError parse_tokens(ParsingContext *context, size_t num_tokens, char **tokens, Node **out_res);
ParserError parse_input(ParsingContext *ctx, char *input, bool pad_parentheses, Node **out_res);
