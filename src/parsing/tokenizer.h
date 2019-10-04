#include <stdbool.h>
#include "context.h"

bool is_space(char c);
bool is_digit(char c);
bool is_letter(char c);
bool tokenize(ParsingContext *ctx, char *input, size_t max_tokens, size_t *out_num_tokens, char **out_tokens);
