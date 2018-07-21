#pragma once
#include "node.h"
#include "rule.h"
#include "context.h"

void free_tree(Node* node);
void free_matching(Matching matching);
void free_rule(RewriteRule rule);
void free_context(ParsingContext *ctx);
