#pragma once
#include "matching.h"
#include "../util/vector.h"
#include "../util/iterator.h"
#include "../tree/node.h"

typedef struct
{
    Pattern pattern;
    Node *after;
} RewriteRule;

RewriteRule get_rule(Pattern *pattern, Node *after);
void free_rule(RewriteRule rule);
bool apply_rule(Node **tree, const RewriteRule *rule, Evaluation eval);

Vector get_empty_ruleset();
void add_to_ruleset(Vector *rules, RewriteRule rule);
void free_ruleset(Vector *rules);
size_t apply_ruleset(Node **tree, const Vector *ruleset, Evaluation eval, size_t cap);
size_t apply_ruleset_by_iterator(Node **tree, Iterator *iterator, Evaluation eval, size_t cap);
