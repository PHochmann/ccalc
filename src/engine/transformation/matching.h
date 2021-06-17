#pragma once
#include "../../util/vector.h"
#include "../tree/node.h"

#define MAX_MAPPED_VARS          10
#define MATCHING_MAX_CONSTRAINTS  3 // Maximum number of constraints per trigger index
#define MATCHING_LIST_PREFIX '['

#define MAX_MAPPED_VARS_EXCEEDED          -1
#define MATCHING_MAX_CONSTRAINTS_EXCEEDED -2

typedef struct
{
    Node *pattern;
    size_t num_constraints[MAX_MAPPED_VARS];
    Node *constraints[MAX_MAPPED_VARS][MATCHING_MAX_CONSTRAINTS];
} Pattern;

/*
Summary: Contains successful matching
*/
typedef struct
{
    NodeList mapped_nodes[MAX_MAPPED_VARS]; // Subtrees in matched_tree that need to replace each mapped_var
} Matching;

typedef bool (*ConstraintChecker)(Node **tree);

NodeList *lookup_mapped_var(const Matching *matching, const char *var);
size_t get_all_matchings(const Node **tree, const Pattern *pattern, ConstraintChecker checker, Matching **out_matchings);
bool get_matching(const Node **tree, const Pattern *pattern, ConstraintChecker checker, Matching *out_matching);
Node **find_matching(const Node **tree, const Pattern *pattern, ConstraintChecker checker, Matching *out_matching);

int get_pattern(Node *tree, size_t num_constraints, Node **constrs, Pattern *out_pattern);
void free_pattern(Pattern *pattern);
