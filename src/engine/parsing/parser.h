#pragma once
#include "../tree/node.h"
#include "../../util/vector.h"
#include "context.h"

typedef enum {
    PERR_NULL,                     // Empty ParsingResult, parser not invoked yet
    PERR_SUCCESS,                  // No error
    PERR_ARGS_MALFORMED,           // NULL-pointer in arguments
    PERR_EXPECTED_INFIX,           // Only without glue-op: Two expressions next to each other
    PERR_UNEXPECTED_INFIX,         // e.g. sum(*)
    PERR_EXCESS_OPENING_PAREN,     // There are opening parenthesis on op-stack after all tokens are processed
    PERR_UNEXPECTED_CLOSING_PAREN, // Mismatch: no opening parenthesis found
    PERR_UNEXPECTED_DELIMITER,     // Delimiter too soon (e.g. sum(,)) or not within parameter list of a function
    PERR_FUNCTION_WRONG_ARITY,     // Function of wrong arity
    PERR_UNEXPECTED_END_OF_EXPR,   // Expression ended too early
    PERR_EXPECTED_PARAM_LIST,      // Function with arity > 0 has been parsed, but no opening parenthesis followed
    PERR_UNEXPECTED_CHARACTER      // Non-alphabet variable
} ParserErrorType;

typedef struct {
    ParserErrorType type;
    size_t error_token;
    int additional_data[2]; // Currently only used for PERR_FUNCTION_WRONG_ARITY to record input arity
} ParserError;

typedef struct {
    Vector tokens;
    ParserError error;
    Node *tree;
} ParsingResult;

ParserError parse_tokens(const ParsingContext *ctx,
    size_t num_tokens,
    const char **tokens,
    Node **out_res);
bool parse_input(const ParsingContext *ctx, const char *input, ParsingResult *out_res);
Node *parse_easy(const ParsingContext *ctx, const char *input);
void free_result(ParsingResult *result, bool also_free_tree);
