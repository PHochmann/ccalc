#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "matching.h"
#include "../util/vector.h"
#include "../tree/tree_util.h"
#include "../util/console_util.h"
#include "../util/string_util.h"

#define VECTOR_STARTSIZE 1

// A suffix-tree is used to lookup common prefixes of matchings
typedef struct {
    size_t first_match_index; // Index of first matching within suffixes, SIZE_MAX denotes pending computation
    size_t num_matchings;     // Amount of matchings of node
    size_t label;             // Amount of tree_children mapped to
    size_t sum;               // Amount of tree_children mapped to before
    size_t distance;          // Number of suffix nodes before
    size_t parent_index;      // Index of parent node within suffix-vector
} SuffixNode;

void fill_suffix(size_t curr_index,
    Node **pattern_children,
    Node **tree_children,
    Vector *matchings,
    Vector *suffixes,
    MappingFilter filter)
{
    if (((SuffixNode*)vec_get(suffixes, curr_index))->first_match_index != SIZE_MAX) return;

    // Recursively fill all suffix nodes before
    fill_suffix(((SuffixNode*)vec_get(suffixes, curr_index))->parent_index,
        pattern_children,
        tree_children,
        matchings,
        suffixes,
        filter);

    // We can save curr for readability and performance because we won't insert into the vector
    SuffixNode *curr = (SuffixNode*)vec_get(suffixes, curr_index);
    curr->first_match_index = vec_count(matchings);

    for (size_t i = 0; i < ((SuffixNode*)vec_get(suffixes, curr->parent_index))->num_matchings; i++)
    {
        // Extend every single matching in parent with additional parameters of this node
        extend_matching(
            *(Matching*)vec_get(matchings, ((SuffixNode*)vec_get(suffixes, curr->parent_index))->first_match_index + i),
            pattern_children[curr->distance - 1],
            (NodeList){ .size = curr->label, .nodes = tree_children + curr->sum },
            matchings,
            filter);
    }

    curr->num_matchings = vec_count(matchings) - curr->first_match_index;
}

void match_parameter_lists(Matching matching,
    size_t num_pattern_children,
    Node **pattern_children,
    size_t num_tree_children,
    Node **tree_children,
    Vector *out_matchings,
    MappingFilter filter)
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
        SuffixNode curr = VEC_GET_ELEM(&vec_suffixes, SuffixNode, curr_index);
        size_t new_sum = curr.sum + curr.label;

        // We found a valid end node of suffixes. Try to match.
        if (new_sum == num_tree_children && curr.distance == num_pattern_children)
        {
            fill_suffix(curr_index,
                pattern_children,
                tree_children,
                &vec_local_matchings,
                &vec_suffixes,
                filter);

            // Update curr
            curr = VEC_GET_ELEM(&vec_suffixes, SuffixNode, curr_index);

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

bool nodelists_equal(NodeList *a, NodeList *b)
{
    if (a->size != b->size) return false;
    for (size_t i = 0; i < a->size; i++)
    {
        if (tree_compare(a->nodes[i], b->nodes[i]) != NULL) return false;
    }
    return true;
}

void extend_matching(Matching matching,
    Node *pattern,
    NodeList tree_list,
    Vector *out_matchings,
    MappingFilter filter)
{
    switch (get_type(pattern))
    {
        // 1. Check if variable is bound, if it is, check occurrence. Otherwise, bind.
        case NTYPE_VARIABLE:
        {
            char *var_name = get_var_name(pattern);

            // Check for wildcard
            if (var_name[0] == MATCHING_WILDCARD
                || (var_name[0] == MATCHING_LIST_PREFIX && var_name[1] == MATCHING_WILDCARD))
            {
                vec_push(out_matchings, &matching);
                return;
            }

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
                // Check with filter if it's okay to bind
                if (filter != NULL && !filter(var_name, tree_list))
                {
                    return;
                }

                matching.mapped_vars[id]  = var_name;
                matching.mapped_nodes[id] = tree_list;
                matching.num_mapped++;
                vec_push(out_matchings, &matching);
                return;
            }
        }
            
        // 2. Check constants for equality
        case NTYPE_CONSTANT:
        {
            if (tree_list.size == 1 && tree_compare(pattern, tree_list.nodes[0]) == NULL)
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
                && get_op(pattern) == get_op(tree_list.nodes[0]))
            {
                match_parameter_lists(matching,
                    get_num_children(pattern),
                    get_child_addr(pattern, 0),
                    get_num_children(tree_list.nodes[0]),
                    get_child_addr(tree_list.nodes[0], 0),
                    out_matchings,
                    filter);
            }
        }
    }
}

size_t get_all_matchings(Node **tree, Node *pattern, Matching **out_matchings, MappingFilter filter)
{
    if (tree == NULL || pattern == NULL || out_matchings == NULL) return false;

    // Due to exponentially many partitions, a lot of states can occur. Use heap.
    Vector result = vec_create(sizeof(Matching), VECTOR_STARTSIZE);

    extend_matching(
        (Matching){ .num_mapped = 0 },
        pattern,
        (NodeList){ .size = 1, .nodes = tree },
        &result,
        filter);
    *out_matchings = (Matching*)result.buffer;

    return result.elem_count;
}

/*
Summary: suffixess to match "tree" against "pattern" (only in root)
Returns: True, if matching is found, false if NULL-pointers given in arguments or no matching found
*/
bool get_matching(Node **tree, Node *pattern, Matching *out_matching, MappingFilter filter)
{
    if (tree == NULL || pattern == NULL || out_matching == NULL) return false;

    Matching *matchings;
    size_t num_matchings = get_all_matchings(tree, pattern, &matchings, filter);

    // Return first matching if any
    if (num_matchings > 0)
    {
        *out_matching = matchings[0];
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
Node **find_matching(Node **tree, Node *pattern, Matching *out_matching, MappingFilter filter)
{
    if (get_matching(tree, pattern, out_matching, filter)) return tree;
    if (get_type(*tree) == NTYPE_OPERATOR)
    {
        for (size_t i = 0; i < get_num_children(*tree); i++)
        {
            Node **res = find_matching(get_child_addr(*tree, i), pattern, out_matching, filter);
            if (res != NULL) return res;
        }
    }
    return NULL;
}

/*
Summary: Basically the same as find_matching, but discards matching
*/
Node **find_matching_discarded(Node *tree, Node *pattern, MappingFilter filter)
{
    Matching matching;
    return find_matching(&tree, pattern, &matching, filter);
}

/*
Summary: Sets ids of variable nodes for faster lookup while matching
*/
void preprocess_pattern(Node *tree)
{
    size_t num_vars = count_variables(tree);
    char *var_names[num_vars];
    size_t num_vars_distinct = list_variables(tree, num_vars, var_names);

    size_t counter = 0;
    for (size_t i = 0; i < num_vars_distinct; i++)
    {
        if (var_names[i][0] != MATCHING_WILDCARD
            && (var_names[i][0] != MATCHING_LIST_PREFIX || var_names[i][1] != MATCHING_WILDCARD))
        {
            if (counter == MAX_MAPPED_VARS)
            {
                report_error("Trying to preprocess a pattern with too many distinct variables. Increase MAX_MAPPED_VARS.\n");
                return;
            }

            Node **nodes[num_vars];
            size_t num_nodes = get_variable_nodes(&tree, var_names[i], nodes);
            for (size_t j = 0; j < num_nodes; j++)
            {
                set_id(*(nodes[j]), counter);
            }
            counter++;
        }
    }
}
