#include <stdbool.h>
#include "context.h"

bool tokenize(ParsingContext *ctx, char *input, size_t max_tokens, size_t *out_num_tokens, char **out_tokens);
bool is_letter(char c);
