#pragma once
#include "../util/vector.h"
#include "../tree/node.h"

#define MAX_MAPPED_VARS 10
#define MATCHING_MAX_CONSTRAINTS 10
#define MATCHING_LIST_PREFIX '['
#define MATCHING_WILDCARD    '_'

typedef struct
{
    Node *pattern;
    size_t num_free_vars;
    const char *free_vars[MAX_MAPPED_VARS];
    size_t num_constraints[MAX_MAPPED_VARS];
    Node *constraints[MAX_MAPPED_VARS][MATCHING_MAX_CONSTRAINTS];
} Pattern;

/*
Summary: Contains successful matching
*/
typedef struct
{
    NodeList mapped_nodes[MAX_MAPPED_VARS];   // Subtrees in matched_tree that need to replace each mapped_var
} Matching;

typedef bool (*ConstraintChecker)(Node **tree);

NodeList *lookup_mapped_var(const Matching *matching, const char *var);
size_t get_all_matchings(const Node **tree, const Pattern *pattern, ConstraintChecker checker, Matching **out_matchings);
bool get_matching(const Node **tree, const Pattern *pattern, ConstraintChecker checker, Matching *out_matching);
Node **find_matching(const Node **tree, const Pattern *pattern, ConstraintChecker checker, Matching *out_matching);
bool does_match(const Node *tree, const Pattern *pattern, ConstraintChecker checker);

Pattern get_pattern(Node *tree, size_t num_constraints, Node **constrs);
void free_pattern(Pattern *pattern);
