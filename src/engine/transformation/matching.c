#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "matching.h"
#include "transformation.h"
#include "../util/vector.h"
#include "../tree/tree_util.h"
#include "../util/console_util.h"
#include "../util/string_util.h"
#include "../tree/tree_to_string.h"

#define VECTOR_STARTSIZE 1

// On stack during matching
typedef struct {
    const Pattern *pattern;
    ConstraintChecker checker;
} MatchingContext;

// A suffix-tree is used to lookup common prefixes of matchings
typedef struct {
    size_t first_match_index; // Index of first matching within suffixes, SIZE_MAX denotes pending computation
    size_t num_matchings;     // Amount of matchings of node
    size_t label;             // Amount of tree_children mapped to
    size_t sum;               // Amount of tree_children mapped to before
    size_t distance;          // Number of suffix nodes before
    size_t parent_index;      // Index of parent node within suffix-vector
} SuffixNode;

static void extend_matching(MatchingContext *ctx,
    Matching matching,
    const Node *pattern,
    NodeList tree_list,
    Vector *out_matchings);

static void fill_suffix(
    MatchingContext *ctx,
    size_t curr_index,
    const Node **pattern_children,
    const Node **tree_children,
    Vector *matchings,
    Vector *suffixes)
{
    // Check if node has already been computed
    if (((SuffixNode*)vec_get(suffixes, curr_index))->first_match_index != SIZE_MAX) return;

    // Recursively fill all suffix nodes before
    fill_suffix(ctx,
        ((SuffixNode*)vec_get(suffixes, curr_index))->parent_index,
        pattern_children,
        tree_children,
        matchings,
        suffixes);

    // We can save curr for readability and performance because we won't insert into the vector
    SuffixNode *curr = (SuffixNode*)vec_get(suffixes, curr_index);
    curr->first_match_index = vec_count(matchings);

    for (size_t i = 0; i < ((SuffixNode*)vec_get(suffixes, curr->parent_index))->num_matchings; i++)
    {
        // Extend every single matching in parent with additional parameters of this node
        extend_matching(
            ctx,
            *(Matching*)vec_get(matchings, ((SuffixNode*)vec_get(suffixes, curr->parent_index))->first_match_index + i),
            pattern_children[curr->distance - 1],
            (NodeList){ .size = curr->label, .nodes = tree_children + curr->sum },
            matchings);
    }

    curr->num_matchings = vec_count(matchings) - curr->first_match_index;
}

static void match_parameter_lists(
    MatchingContext *ctx,
    Matching matching,
    size_t num_pattern_children,
    const Node **pattern_children,
    size_t num_tree_children,
    const Node **tree_children,
    Vector *out_matchings)
{
    Vector vec_local_matchings = vec_create(sizeof(Matching), VECTOR_STARTSIZE);
    Vector vec_suffixes = vec_create(sizeof(SuffixNode), VECTOR_STARTSIZE);

    vec_push(&vec_local_matchings, &matching);
    VEC_PUSH_ELEM(&vec_suffixes, SuffixNode, ((SuffixNode){
        .first_match_index = 0,
        .num_matchings     = 1,
        .sum               = 0,
        .distance          = 0,
        .label             = 0,
        .parent_index      = 0  // Should never be used
    }));

    size_t curr_index = 0;
    while (curr_index < vec_count(&vec_suffixes))
    {
        // Since vec_suffixes's buffer could be realloced by any insertion, we can't store a pointer to it
        SuffixNode curr = *(SuffixNode*)vec_get(&vec_suffixes, curr_index);
        size_t new_sum = curr.sum + curr.label;

        // We found a valid end node of suffixes. Try to match.
        if (new_sum == num_tree_children && curr.distance == num_pattern_children)
        {
            fill_suffix(ctx,
                curr_index,
                pattern_children,
                tree_children,
                &vec_local_matchings,
                &vec_suffixes);

            // Update curr
            curr = *(SuffixNode*)vec_get(&vec_suffixes, curr_index);

            // Copy matchings to result-buffer
            vec_push_many(out_matchings,
                vec_count(&vec_local_matchings) - curr.first_match_index,
                vec_get(&vec_local_matchings, curr.first_match_index));
        }

        // Extend entry of suffixes
        if (curr.distance < num_pattern_children)
        {
            if (get_type(pattern_children[curr.distance]) == NTYPE_VARIABLE
                && get_var_name(pattern_children[curr.distance])[0] == MATCHING_LIST_PREFIX)
            {
                // Current pattern-child is list-variable
                NodeList *list = &matching.mapped_nodes[get_id(pattern_children[curr.distance])];
                if (list->nodes != NULL && new_sum + 1 <= num_tree_children)
                {
                    // List is already bound, thus also its length is bound
                    VEC_PUSH_ELEM(&vec_suffixes, SuffixNode, ((SuffixNode){
                        .first_match_index = SIZE_MAX,
                        .num_matchings     = 0,
                        .label             = list->size,
                        .sum               = new_sum,
                        .distance          = curr.distance + 1,
                        .parent_index      = curr_index
                    }));
                }
                else
                {
                    if (curr.distance == num_pattern_children - 1)
                    {
                        // Special case: List is last pattern-child
                        // We can avoid extending suffixes with lists that are too short
                        VEC_PUSH_ELEM(&vec_suffixes, SuffixNode, ((SuffixNode){
                                .first_match_index = SIZE_MAX,
                                .num_matchings     = 0,
                                .label             = num_tree_children - new_sum,
                                .sum               = new_sum,
                                .distance          = curr.distance + 1,
                                .parent_index      = curr_index
                        }));
                    }
                    else
                    {
                        // List is not bound yet, suffixes needs to be extended with all possible lengths
                        size_t num_insertions = num_tree_children - new_sum + 1;
                        for (size_t i = 0; i < num_insertions; i++)
                        {
                            VEC_PUSH_ELEM(&vec_suffixes, SuffixNode, ((SuffixNode){
                                .first_match_index = SIZE_MAX,
                                .num_matchings     = 0,
                                .label             = i,
                                .sum               = new_sum,
                                .distance          = curr.distance + 1,
                                .parent_index      = curr_index
                            }));
                        }
                    }
                }
            }
            else
            {
                if (new_sum < num_tree_children)
                {
                    // Any non-list node in pattern corresponds to exactly one node in tree
                    VEC_PUSH_ELEM(&vec_suffixes, SuffixNode, ((SuffixNode){
                        .first_match_index = SIZE_MAX,
                        .num_matchings     = 0,
                        .label             = 1,
                        .sum               = new_sum,
                        .distance          = curr.distance + 1,
                        .parent_index      = curr_index
                    }));
                }
            }
        }
        curr_index++;
    }
    
    vec_destroy(&vec_local_matchings);
    vec_destroy(&vec_suffixes);
}

static bool nodelists_equal(const NodeList *a, const NodeList *b)
{
    if (a->size != b->size) return false;
    for (size_t i = 0; i < a->size; i++)
    {
        if (!tree_equals(a->nodes[i], b->nodes[i])) return false;
    }
    return true;
}

static void extend_matching(
    MatchingContext *ctx,
    Matching matching,
    const Node *pattern,
    NodeList tree_list,
    Vector *out_matchings)
{
    switch (get_type(pattern))
    {
        // 1. Check if variable is bound, if it is, check occurrence. Otherwise, bind.
        case NTYPE_VARIABLE:
        {
            size_t id = get_id(pattern);
            NodeList *nodes = &matching.mapped_nodes[id];
            if (nodes->nodes != NULL) // Already bound
            {
                // Is already bound variable equal to this occurrence?
                if (nodelists_equal(nodes, &tree_list))
                {
                    vec_push(out_matchings, &matching);
                }
                return;
            }
            else
            {
                matching.mapped_nodes[id] = tree_list;

                // Check constraints if its okay to bind
                for (size_t i = 0; i < ctx->pattern->num_constraints[id]; i++)
                {
                    Node *constr_cpy = tree_copy(ctx->pattern->constraints[id][i]);
                    transform_by_matching(id + 1, ctx->pattern->free_vars, &matching, &constr_cpy);
                    bool res = ctx->checker(&constr_cpy);
                    free_tree(constr_cpy);
                    if (!res) return;
                }

                vec_push(out_matchings, &matching);
                return;
            }
        }
            
        // 2. Check constants for equality
        case NTYPE_CONSTANT:
        {
            if (tree_list.size == 1 && tree_equals(pattern, tree_list.nodes[0]))
            {
                vec_push(out_matchings, &matching);
            }
            return;
        }
            
        // 3. Check operator and arity for equality
        case NTYPE_OPERATOR:
        {
            if (tree_list.size == 1
                && get_type(tree_list.nodes[0]) == NTYPE_OPERATOR
                && get_op(pattern)->id == get_op(tree_list.nodes[0])->id)
            {
                match_parameter_lists(ctx,
                    matching,
                    get_num_children(pattern),
                    (const Node**)get_child_addr(pattern, 0),
                    get_num_children(tree_list.nodes[0]),
                    (const Node**)get_child_addr(tree_list.nodes[0], 0),
                    out_matchings);
            }
        }
    }
}

/*
Summary: Generates all possible matchings
Params
    tree: Tree that is checked for pattern occurrence.
    pattern: Tree and constraints that are used as pattern (including list-variables).
    checker: Callback that is invoked when a constraint is checked.
    out_matchings: Heap-pointer to generated matchings will be placed here. Is allowed to be NULL.
Returns: Number of matchings found
*/
size_t get_all_matchings(const Node **tree, const Pattern *pattern, ConstraintChecker checker, Matching **out_matchings)
{
    if (tree == NULL || pattern == NULL) return false;

    // Create context object and pass by pointer, this saves stack space during recursion
    MatchingContext ctx = (MatchingContext){
        .pattern = pattern,
        .checker = checker
    };
    // Due to exponentially many partitions of parameter lists, a lot of partial matchings can occur. Use heap.
    Vector result = vec_create(sizeof(Matching), VECTOR_STARTSIZE);

    extend_matching(
        &ctx,
        (Matching){ .mapped_nodes = { { .size = 0, .nodes = NULL } } },
        pattern->pattern,
        (NodeList){ .size = 1, .nodes = tree },
        &result);
    
    if (out_matchings != NULL)
    {
        *out_matchings = (Matching*)result.buffer;
    }
    else
    {
        vec_destroy(&result);
    }
    
    return result.elem_count;
}

/*
Summary: suffixess to match "tree" against "pattern" (only in root)
Params
    tree, pattern, checker: As in get_all_matchings
    out_matching:           Location were first matching will be placed. Is allowed to be NULL.
Returns: True, if matching is found, false if no matching found
*/
bool get_matching(const Node **tree, const Pattern *pattern, ConstraintChecker checker, Matching *out_matching)
{
    if (tree == NULL || pattern == NULL) return false;

    Matching *matchings;
    size_t num_matchings = get_all_matchings(tree, pattern, checker, &matchings);

    // Return first matching if any
    if (num_matchings > 0)
    {
        if (out_matching != NULL)
        {
            *out_matching = matchings[0];
        }
        free(matchings);
        return true;
    }
    else
    {
        free(matchings);
        return false;
    }
}

/*
Summary: Looks for matching in tree, i.e. suffixess to construct matching in each node until matching is found (Top-Down)
*/
Node **find_matching(const Node **tree, const Pattern *pattern, ConstraintChecker checker, Matching *out_matching)
{
    if (get_matching(tree, pattern, checker, out_matching)) return (Node**)tree;
    if (get_type(*tree) == NTYPE_OPERATOR)
    {
        for (size_t i = 0; i < get_num_children(*tree); i++)
        {
            Node **res = find_matching((const Node**)get_child_addr(*tree, i), pattern, checker, out_matching);
            if (res != NULL) return res;
        }
    }
    return NULL;
}

/*
Summary: - Sets ids of variable nodes for faster lookup while matching
         - Computes trigger-indices for constraints
Returns: 0 if success
         -1 if MAX_MAPPED_VARS exceeded
         -2 if MAX_VARIABLE_OCCURRANCES exceeded
         -3 if MATCHING_MAX_CONSTRAINTS exceeded
*/
int get_pattern(Node *tree, size_t num_constraints, Node **constrs, Pattern *out_pattern)
{
    *out_pattern = (Pattern){
        .pattern         = tree,
        .num_free_vars   = 0,
        .num_constraints = { 0 }
    };

    bool sufficient = false;
    (*out_pattern).num_free_vars = list_variables(tree, MAX_MAPPED_VARS, (*out_pattern).free_vars, &sufficient);

    if (!sufficient)
    {
        return MAX_MAPPED_VARS_EXCEEDED;
    }

    // Step 1: Set id in pattern tree
    for (size_t i = 0; i < (*out_pattern).num_free_vars; i++)
    {
        Node **nodes[MAX_VARIABLE_OCURRANCES];
        size_t num_nodes = get_variable_nodes((const Node**)&tree, (*out_pattern).free_vars[i], MAX_VARIABLE_OCURRANCES, nodes);

        if (num_nodes > MAX_VARIABLE_OCURRANCES)
        {
            return MAX_VARIABLE_OCCURRANCES_EXCEEDED;
        }

        for (size_t j = 0; j < num_nodes; j++)
        {
            set_id(*(nodes[j]), i);
        }
    }

    // Step 2: Compute contraints that should be checked when variable with this id is bound
    // This involves computing the variable with the highest id that occurs in the constraint
    for (size_t i = 0; i < num_constraints; i++)
    {
        size_t max_id = 0;
        for (size_t j = 0; j < (*out_pattern).num_free_vars; j++)
        {
            if (get_variable_nodes((const Node**)&constrs[i], (*out_pattern).free_vars[j], 0, NULL) != 0)
            {
                max_id = j;
            }
        }

        if ((*out_pattern).num_constraints[max_id] == MATCHING_MAX_CONSTRAINTS)
        {
            return MATCHING_MAX_CONSTRAINTS_EXCEEDED;
        }

        // max_id contains trigger index
        (*out_pattern).constraints[max_id][(*out_pattern).num_constraints[max_id]] = constrs[i];
        (*out_pattern).num_constraints[max_id]++;
    }
    return 0;
}

void free_pattern(Pattern *pattern)
{
    free_tree(pattern->pattern);
    for (size_t i = 0; i < MAX_MAPPED_VARS; i++)
    {
        for (size_t j = 0; j < pattern->num_constraints[i]; j++)
        {
            free_tree(pattern->constraints[i][j]);
        }
    }
}
