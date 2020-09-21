#include <stdbool.h>
#include "context.h"
#include "../util/vector.h"

void tokenize(const char *input, const Trie *keywords_trie, Vector *out_tokens);
void free_tokens(Vector *tokens);
