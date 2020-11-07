#pragma once
#include "../util/vector.h"
#include "../tree/node.h"

#define MAX_MAPPED_VARS 10
#define MATCHING_LIST_PREFIX '['
#define MATCHING_WILDCARD    '_'

/*
Summary: Contains successful or intermediate matching
*/
typedef struct
{
    size_t num_mapped;                      // Size of mapped_vars and mapped_nodes
    const char *mapped_vars[MAX_MAPPED_VARS];     // Variables in pattern, not copied, thus lifetime coupled to RewriteRule or whatever supplied the pattern
    NodeList mapped_nodes[MAX_MAPPED_VARS]; // Subtrees in matched_tree that need to replace each mapped_var
} Matching;

typedef bool (*MappingFilter)(const char *var, NodeList nodes, Matching *matching);

NodeList *lookup_mapped_var(const Matching *matching, const char *var);
size_t get_all_matchings(const Node **tree, const Node *pattern, Matching **out_matchings, MappingFilter filter);
bool get_matching(const Node **tree, const Node *pattern, Matching *out_matching, MappingFilter filter);
Node **find_matching(const Node **tree, const Node *pattern, Matching *out_matching, MappingFilter filter);
bool does_match(const Node *tree, const Node *pattern, MappingFilter filter);
void preprocess_pattern(Node *tree);
