#pragma once
#include "matching.h"
#include "../util/vector.h"
#include "../tree/node.h"

typedef struct
{
    Node *before;
    Node *after;
    MappingFilter filter;
} RewriteRule;

RewriteRule get_rule(Node *before, Node *after, MappingFilter filter);
void free_rule(RewriteRule rule);
bool apply_rule(Node **tree, RewriteRule *rule);

Vector get_empty_ruleset();
void add_to_ruleset(Vector *rules, RewriteRule rule);
void free_ruleset(Vector *rules);
int apply_ruleset(Node **tree, Vector *rules);
bool parse_rulesets(FILE *file,
    ParsingContext *ctx,
    MappingFilter default_filter,
    size_t buffer_size,
    Vector *out_rulesets);
