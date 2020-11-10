#pragma once
#include "../util/vector.h"
#include "../tree/node.h"

#define MAX_MAPPED_VARS 10
#define MATCHING_MAX_CONSTRAINTS 10
#define MATCHING_LIST_PREFIX '['
#define MATCHING_WILDCARD    '_'

typedef struct
{
    Node *lhs;
    Node *rhs;
} PatternConstraint;

typedef struct
{
    Node *pattern;
    size_t num_constraints[MAX_MAPPED_VARS];
    PatternConstraint constraints[MAX_MAPPED_VARS][MATCHING_MAX_CONSTRAINTS];
} Pattern;

/*
Summary: Contains successful or intermediate matching
*/
typedef struct
{
    size_t num_mapped;                        // Size of mapped_vars and mapped_nodes
    const char *mapped_vars[MAX_MAPPED_VARS]; // Variables in pattern, not copied, thus lifetime coupled to RewriteRule or whatever supplied the pattern
    NodeList mapped_nodes[MAX_MAPPED_VARS];   // Subtrees in matched_tree that need to replace each mapped_var
} Matching;

NodeList *lookup_mapped_var(const Matching *matching, const char *var);
size_t get_all_matchings(const Node **tree, const Pattern *pattern, TreeListener listener, Matching **out_matchings);
bool get_matching(const Node **tree, const Pattern *pattern, TreeListener listener, Matching *out_matching);
Node **find_matching(const Node **tree, const Pattern *pattern, TreeListener listener, Matching *out_matching);
bool does_match(const Node *tree, const Pattern *pattern, TreeListener listener);

Pattern get_pattern(Node *tree, size_t num_constraints, PatternConstraint *constrs);
void free_pattern(Pattern *pattern);
