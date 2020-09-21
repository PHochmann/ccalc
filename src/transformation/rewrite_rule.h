#pragma once
#include "matching.h"
#include "../util/vector.h"
#include "../util/iterator.h"
#include "../tree/node.h"

typedef Vector Ruleset;

typedef struct
{
    Node *before;
    Node *after;
    MappingFilter filter;
} RewriteRule;

RewriteRule get_rule(Node *before, Node *after, MappingFilter filter);
void free_rule(RewriteRule rule);
bool apply_rule(Node **tree, const RewriteRule *rule);

Ruleset get_empty_ruleset();
void add_to_ruleset(Ruleset *rules, RewriteRule rule);
void free_ruleset(Ruleset *rules);
size_t apply_ruleset(Node **tree, const Ruleset *ruleset);
size_t apply_ruleset_by_iterator(Node **tree, Iterator *iterator);
