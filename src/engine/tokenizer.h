#include <stdbool.h>
#include "context.h"

extern const size_t MAX_TOKENS;

bool tokenize(ParsingContext *ctx, char *input, size_t *out_num_tokens, char **out_tokens);
bool is_space(char c);
bool is_letter(char c);
bool is_digit(char c);
bool is_opening_parenthesis(char c);
bool is_closing_parenthesis(char c);
bool is_delimiter(char c);
bool is_precedence_joker(char c);
