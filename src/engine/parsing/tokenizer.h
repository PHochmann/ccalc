#pragma once
#include <stdbool.h>
#include "context.h"
#include "../../util/vector.h"

Vector tokenize(const char *input, const Trie *keywords_trie);
void free_tokens(Vector *tokens);
