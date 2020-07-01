#pragma once
#include "../util/vector.h"
#include "../tree/node.h"

#define MAX_MAPPED_VARS 10

// Special rule prefixes:
#define MATCHING_LIST_PREFIX        '['
#define MATCHING_CONST_PREFIX       'c'
#define MATCHING_NON_CONST_PREFIX   'd'
#define MATCHING_VAR_PREFIX         'v'
#define MATCHING_NON_VAR_PREFIX     'w'
#define MATCHING_LITERAL_VAR_PREFIX '_'

/*
Summary: Contains successful or intermediate matching
*/
typedef struct
{
    size_t num_mapped;                      // Size of mapped_vars and mapped_nodes
    char *mapped_vars[MAX_MAPPED_VARS];     // Variables in pattern, not copied, thus lifetime coupled to RewriteRule or whatever supplied the pattern
    NodeList mapped_nodes[MAX_MAPPED_VARS]; // Subtrees in matched_tree that need to replace each mapped_var
} Matching;

NodeList *lookup_mapped_var(Matching *matching, char *var);
void extend_matching(Matching matching, Node *pattern, NodeList tree_list, Vector *out_matchings);
size_t get_all_matchings(Node **tree, Node *pattern, Matching **out_matchings);
bool get_matching(Node **tree, Node *pattern, Matching *out_matching);
Node **find_matching(Node **tree, Node *pattern, Matching *out_matching);
Node **find_matching_discarded(Node *tree, Node *pattern);
