#pragma once
#include "../tree/node.h"

/*
Contains successful matching
*/
typedef struct
{
    // Subtree that could be matched, double indirection to replace it
    Node **matched_tree;
    // Size of mapped_vars and mapped_nodes
    size_t num_mapped;
    // Variables in pattern (on heap)
    // Not copied, thus lifetime coupled to RewriteRule or whatever supplied the pattern
    char **mapped_vars;
    // Subtrees in matched_tree that need to replace each mapped_var (on heap)
    Node **mapped_nodes;
} Matching;

bool get_matching(Node **tree, Node *pattern, Matching *out_matching);
void free_matching(Matching matching);
bool find_matching(Node **tree, Node *pattern, Matching *out_matching);
bool find_matching_discarded(Node *tree, Node *pattern);
