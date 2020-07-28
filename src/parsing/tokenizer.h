#include <stdbool.h>
#include "context.h"
#include "../util/vector.h"

void tokenize(char *input, Trie *keywords_trie, Vector *out_tokens);
void free_tokens(Vector *tokens);
