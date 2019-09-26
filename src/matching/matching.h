#pragma once
#include "../parsing/context.h"
#include "../parsing/node.h"

/*
Contains successful matching
*/
typedef struct {
    Node *matched_tree;  // Subtree that could be matched
    char **mapped_vars;  // Variables in pattern
    Node **mapped_nodes; // Subtrees in matched_tree that need to replace each mapped_var
    size_t num_mapped;   // Size of mapped_vars and mapped_nodes
} Matching;

typedef bool *(*MatchHandler)(ParsingContext *ctx, Matching matching);

bool get_matching(ParsingContext *ctx, Node *tree, Node *pattern, Matching *out_matching);
void free_matching(Matching matching);
bool find_matching(ParsingContext *ctx, Node *tree, Node *pattern, Matching *out_matching);
bool find_matching_discarded(ParsingContext *ctx, Node *tree, Node *pattern);
