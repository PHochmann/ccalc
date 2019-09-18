#pragma once
#include "../parsing/context.h"
#include "../parsing/node.h"

typedef struct {
    ParsingContext *context;
    Node *before;
    Node *after;
} RewriteRule;

RewriteRule get_rule(ParsingContext *ctx, Node *before, Node *after);
void free_rule(RewriteRule rule);
bool apply_rule(Node *tree, RewriteRule *rule);

// Experimental
bool infer_rule(ParsingContext *ctx, RewriteRule *rule_a, RewriteRule *rule_b, RewriteRule *out_rule);
size_t apply_ruleset(Node *tree, size_t num_rules, RewriteRule *rules, size_t max_iterations);