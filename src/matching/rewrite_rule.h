#pragma once
#include "matching.h"
#include "../parsing/context.h"
#include "../parsing/node.h"

typedef struct
{
    Node *before;
    Node *after;
} RewriteRule;

RewriteRule get_rule(Node *before, Node *after);
void free_rule(RewriteRule rule);
void transform_matched_by_rule(ParsingContext *ctx, Node *rule_after, Matching *matching);
bool apply_rule(ParsingContext *ctx, Node *tree, RewriteRule *rule);
