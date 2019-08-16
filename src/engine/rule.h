#pragma once
#include "context.h"
#include "node.h"

typedef struct {
    ParsingContext *context;
    Node *before;
    Node *after;
} RewriteRule;

/*
Contains successful matching
*/
typedef struct {
    Node *matched_tree;  // Subtree that could be matched
    char **mapped_vars;  // Variables in pattern
    Node **mapped_nodes; // Subtrees in matched_tree that need to replace each mapped_var
    size_t num_mapped;   // Size of mappes_vars and mapped_nodes
} Matching;

RewriteRule get_rule(ParsingContext *ctx, Node *before, Node *after);
bool get_matching(ParsingContext *ctx, Node *tree, Node *pattern, Matching *out_matching);
void free_matching(Matching matching);
void free_rule(RewriteRule rule);
bool find_matching(ParsingContext *ctx, Node *tree, Node *pattern, Matching *out_matching);
void transform_by_rule(RewriteRule *rule, Matching *matching);
bool apply_rule(Node *tree, RewriteRule *rule);
size_t apply_ruleset(Node *tree, size_t num_rules, RewriteRule *rules, size_t max_iterations);
