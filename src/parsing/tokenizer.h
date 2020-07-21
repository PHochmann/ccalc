#include <stdbool.h>
#include "context.h"
#include "../util/vector.h"

void init_tokenizer(ParsingContext *ctx);
void unload_tokenizer();
void tokenize(char *input, Vector *out_tokens);
void free_tokens(Vector *tokens);
