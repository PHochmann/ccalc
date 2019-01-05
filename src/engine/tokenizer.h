#include <stdbool.h>

#include "constants.h"
#include "context.h"

bool tokenize(ParsingContext *ctx, bool pad_parentheses, char *input, size_t *out_num_tokens, char **out_tokens);

bool is_space(char c);
bool is_letter(char c);
bool is_digit(char c);
bool is_opening_parenthesis(char c);
bool is_closing_parenthesis(char c);
bool is_delimiter(char c);
