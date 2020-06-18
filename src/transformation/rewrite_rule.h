#pragma once
#include "matching.h"
#include "../tree/node.h"

typedef struct
{
    Node *before;
    Node *after;
} RewriteRule;

RewriteRule get_rule(Node **before, Node **after);
void free_rule(RewriteRule rule);
void transform_matched_by_rule(Node *rule_after, Matching *matching, Node **matched_subtree);
bool apply_rule(Node **tree, RewriteRule *rule);
void apply_ruleset(Node **tree, size_t num_rules, RewriteRule *ruleset);
