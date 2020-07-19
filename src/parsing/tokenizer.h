#include <stdbool.h>
#include "context.h"
#include "../util/vector.h"

void tokenize(ParsingContext *ctx, char *input, Vector *out_tokens);
void free_tokens(Vector *tokens);
