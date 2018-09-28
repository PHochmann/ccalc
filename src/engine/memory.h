#pragma once
#include <stdbool.h>
#include "node.h"
#include "rule.h"
#include "context.h"

void free_tree(Node* node, bool preserve_root);
void free_matching(Matching matching);
void free_rule(RewriteRule rule);
void free_context(ParsingContext *ctx);
