#pragma once
#include "matching.h"
#include "../util/linked_list.h"
#include "../util/vector.h"
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
bool apply_rule(Node **tree, RewriteRule *rule);

Ruleset get_empty_ruleset();
void add_to_ruleset(Ruleset *rules, RewriteRule rule);
void free_ruleset(Ruleset *rules);
size_t apply_ruleset(Node **tree, Ruleset *rules);
size_t apply_rule_list(Node **tree, LinkedList *rules);
