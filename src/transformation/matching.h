#pragma once
#include "../tree/node.h"

#define MAX_MAPPED_VARS 10

/*
Contains successful matching
*/
typedef struct
{
    // Size of mapped_vars and mapped_nodes
    size_t num_mapped;
    // Variables in pattern (on heap)
    // Not copied, thus lifetime coupled to RewriteRule or whatever supplied the pattern
    char *mapped_vars[10];
    // Subtrees in matched_tree that need to replace each mapped_var (on heap)
    NodeList mapped_nodes[10];
} Matching;

NodeList *lookup_mapped_var(Matching *matching, char *var);
size_t extend_matching(Matching matching, Node *pattern, NodeList tree_list, Matching *out_matchings);
bool get_matching(Node *tree, Node *pattern, Matching *out_matching);
Node **find_matching(Node **tree, Node *pattern, Matching *out_matching);
Node **find_matching_discarded(Node *tree, Node *pattern);
