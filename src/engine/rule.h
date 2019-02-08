#pragma once
#include "context.h"
#include "node.h"

#define VAR_PREFIX "var_"
#define CONST_PREFIX "const_"
#define LITERAL_PREFIX "literal_"

typedef struct {
    ParsingContext *context;
    Node *before;
    Node *after;
} RewriteRule;

typedef struct {
    Node *matched_tree;
    char **mapped_vars;
    Node **mapped_nodes;
    size_t num_mapped;
} Matching;

RewriteRule get_rule(ParsingContext *ctx, Node *before, Node *after);
bool get_matching(ParsingContext *ctx, Node *tree, Node *pattern, Matching *out_matching);
void free_matching(Matching matching);
void free_rule(RewriteRule rule);
bool find_matching(ParsingContext *ctx, Node *tree, Node *pattern, Matching *out_matching);
void transform_by_rule(RewriteRule *rule, Matching *matching);
bool apply_rule(Node *tree, RewriteRule *rule);
int apply_ruleset(Node *tree, size_t num_rules, RewriteRule *rules, int max_iterations);
