#pragma once
#include "../tree/node.h"
#include "context.h"

typedef enum {
    PERR_SUCCESS,                    // No error
    PERR_ARGS_MALFORMED,             // NULL-pointer in arguments
    PERR_UNEXPECTED_SUBEXPRESSION,   // Only without glue-op: Two expressions next to each other
    PERR_EXCESS_OPENING_PARENTHESIS, // There are opening parenthesis on op-stack after all tokens are processed
    PERR_UNEXPECTED_CLOSING_PARENTHESIS, // Mismatch: no opening parenthesis found
    PERR_UNEXPECTED_DELIMITER,       // Delimiter too soon (e.g. 1+,) or not within parameter list of a function
    PERR_MISSING_OPERATOR,           // Too many children
    PERR_MISSING_OPERAND,            // Could not pop 'arity' amount of nodes
    PERR_FUNCTION_WRONG_ARITY,       // Function of wrong arity
    PERR_CHILDREN_EXCEEDED,          // Too many operands for function
    PERR_UNEXPECTED_END_OF_EXPR,     // Expression ended too early
    PERR_EMPTY,                      // No node has been created
} ParserError;

ParserError parse_tokens(const ParsingContext *ctx, size_t num_tokens, const char **tokens, Node **out_res, size_t *error_token);
ParserError parse_input(const ParsingContext *ctx, const char *input, Node **out_res, size_t *error_pos);
Node *parse_conveniently(const ParsingContext *ctx, const char *input);
const char *perr_to_string(ParserError perr);
