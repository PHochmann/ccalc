#include <stdbool.h>
#include <string.h>

#include "matching.h"
#include "../tree/tree_util.h"

/*
Todo: protect against buffer overflows
*/

#define MAX_MATCHINGS 5000
#define MAX_PARTITIONS 500

typedef struct TrieNode {
    Matching *matchings;
    size_t num_matchings;
    size_t label;         // Amount of tree-children of this node
    size_t sum;           // Amount of tree-children until this node
    size_t distance;      // Amount of pattern-children until this node
    struct TrieNode *parent;
} TrieNode;

size_t fill_trie(TrieNode *curr,
    Node **pattern_children,
    Node **tree_children,
    size_t buffer_size,
    Matching *matchings_buffer)
{
    if (curr->matchings != NULL) return 0;

    size_t buffer_change = fill_trie(
        curr->parent,
        pattern_children,
        tree_children,
        buffer_size,
        matchings_buffer);

    matchings_buffer += buffer_change;
    buffer_size -= buffer_change;
    curr->matchings = matchings_buffer;

    for (size_t i = 0; i < curr->parent->num_matchings; i++)
    {
        size_t new_num_matchings = extend_matching(
            curr->parent->matchings[i],
            pattern_children[curr->distance - 1],
            (NodeList){ .size = curr->label, .nodes = tree_children + curr->sum },
            buffer_size,
            matchings_buffer);
        
        buffer_size -= new_num_matchings;
        curr->num_matchings += new_num_matchings;
        matchings_buffer += new_num_matchings;
    }

    return buffer_change + curr->num_matchings;
}

NodeList *lookup_mapped_var(Matching *matching, char *var)
{
    for (size_t i = 0; i < matching->num_mapped; i++)
    {
        if (strcmp(matching->mapped_vars[i], var) == 0)
        {
            return &matching->mapped_nodes[i];
        }
    }
    return false;
}

size_t match_parameter_lists(Matching matching,
    size_t num_pattern_children,
    Node **pattern_children,
    size_t num_tree_children,
    Node **tree_children,
    size_t buffer_size,
    Matching *out_matchings)
{
    Matching *local_matchings = malloc(MAX_MATCHINGS * sizeof(Matching));
    size_t num_local_matchings = 0;
    size_t result = 0;

    TrieNode trie[MAX_PARTITIONS];
    trie[0] = (TrieNode){
        .matchings     = &matching,
        .num_matchings = 1,
        .sum           = 0,
        .distance      = 0,
        .label         = 0,
        .parent        = NULL
    };

    size_t trie_size = 1;
    size_t curr = 0;

    while (curr < trie_size)
    {
        size_t new_sum = trie[curr].sum + trie[curr].label;

        // We found a valid end node of trie
        // Try to match
        if (new_sum == num_tree_children && trie[curr].distance == num_pattern_children)
        {
            size_t num = fill_trie(&trie[curr],
                pattern_children,
                tree_children,
                MAX_MATCHINGS - num_local_matchings,
                local_matchings + num_local_matchings);
            num_local_matchings += num;

            // Copy matchings to result-buffer
            if (buffer_size >= trie[curr].num_matchings)
            {
                memcpy(out_matchings + result,
                    trie[curr].matchings,
                    trie[curr].num_matchings * sizeof(Matching));
                result += trie[curr].num_matchings;
                buffer_size -= trie[curr].num_matchings;
            }
        }

        // Extend entry of trie
        if (trie[curr].distance < num_pattern_children)
        {
            if (get_type(pattern_children[trie[curr].distance]) == NTYPE_VARIABLE
                && get_var_name(pattern_children[trie[curr].distance])[0] == MATCHING_LIST_PREFIX)
            {
                NodeList *list = lookup_mapped_var(&matching, get_var_name(pattern_children[trie[curr].distance]));
                if (list != NULL && new_sum + 1 <= num_tree_children)
                {
                    // List is already bound, thus also its length is bound
                    trie[trie_size++] = (TrieNode){
                        .matchings     = NULL,
                        .num_matchings = 0,
                        .label         = list->size,
                        .sum           = new_sum,
                        .distance      = trie[curr].distance + 1,
                        .parent        = &trie[curr]
                    };
                }
                else
                {
                    // List is not bound yet, trie needs to be extended with all possible lengths
                    for (size_t i = 0; new_sum + i <= num_tree_children; i++)
                    {
                        trie[trie_size++] = (TrieNode){
                            .matchings     = NULL,
                            .num_matchings = 0,
                            .label         = i,
                            .sum           = new_sum,
                            .distance      = trie[curr].distance + 1,
                            .parent        = &trie[curr]
                        };
                    }
                }
            }
            else
            {
                if (new_sum < num_tree_children)
                {
                    // Any non-list node in pattern corresponds to exactly one node in tree
                    trie[trie_size++] = (TrieNode){
                        .matchings     = NULL,
                        .num_matchings = 0,
                        .label         = 1,
                        .sum           = new_sum,
                        .distance      = trie[curr].distance + 1,
                        .parent        = &trie[curr]
                    };
                }
            }
        }
        curr++;
    }
    
    free(local_matchings);
    return result;
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

size_t extend_matching(Matching matching,
    Node *pattern,
    NodeList tree_list,
    size_t buffer_size,
    Matching *out_matchings)
{
    if (buffer_size == 0)
    {
        return 0;
    }

    switch (get_type(pattern))
    {
        // 1. Check if variable is bound, if it is, check occurrence. Otherwise, bind.
        case NTYPE_VARIABLE:
        {
            NodeList *nodes = lookup_mapped_var(&matching, get_var_name(pattern));
            if (nodes != NULL) // Already bound
            {
                // Is already bound variable equal to this occurrence?
                // If not, fail here
                if (!nodelists_equal(nodes, &tree_list))
                {
                    return 0;
                }

                out_matchings[0] = matching;
                return 1;
            }
            else
            {
                // Check special rules
                switch (get_var_name(pattern)[0])
                {
                    case MATCHING_CONST_PREFIX:
                        if (tree_list.size != 1
                            || get_type(tree_list.nodes[0]) != NTYPE_CONSTANT) return 0;
                        break;
                    case MATCHING_NON_CONST_PREFIX:
                        if (tree_list.size != 1
                            || get_type(tree_list.nodes[0]) == NTYPE_CONSTANT) return 0;
                }

                // Bind variable
                out_matchings[0] = matching;
                out_matchings[0].mapped_vars[matching.num_mapped]  = get_var_name(pattern);
                out_matchings[0].mapped_nodes[matching.num_mapped] = tree_list;
                out_matchings[0].num_mapped++;
                return 1;
            }
        }
            
        // 2. Check constants for equality
        case NTYPE_CONSTANT:
        {
            if (tree_list.size != 1) return 0;
            Node *comp = tree_compare(pattern, tree_list.nodes[0]);
            if (comp == NULL)
            {
                out_matchings[0] = matching;
                return 1;
            }
            else
            {
                return 0;
            }
        }
            
        // 3. Check operator and arity for equality
        case NTYPE_OPERATOR:
        {
            if (tree_list.size != 1) return 0;
            if (get_type(tree_list.nodes[0]) != NTYPE_OPERATOR
                || get_op(pattern) != get_op(tree_list.nodes[0])
                /*|| get_num_children(curr_pattern) != get_num_children(curr_tree)*/)
            {
                return 0;
            }

            return match_parameter_lists(matching,
                get_num_children(pattern),
                get_child_addr(pattern, 0),
                get_num_children(tree_list.nodes[0]),
                get_child_addr(tree_list.nodes[0], 0),
                buffer_size,
                out_matchings);
        }
    }

    return 0;
}

size_t get_all_matchings(Node **tree, Node *pattern, Matching **out_matchings)
{
    if (tree == NULL || pattern == NULL || out_matchings == NULL) return false;

    // Due to exponential many partitions, a lot of states can occur. Use heap.
    *out_matchings = malloc(MAX_MATCHINGS * sizeof(Matching));
    size_t res = extend_matching(
        (Matching){ .num_mapped = 0 },
        pattern,
        (NodeList){ .size = 1, .nodes = tree },
        MAX_MATCHINGS,
        *out_matchings);

    return res;
}

/*
Summary: Tries to match "tree" against "pattern" (only in root)
Returns: True, if matching is found, false if NULL-pointers given in arguments or no matching found
*/
bool get_matching(Node **tree, Node *pattern, Matching *out_matching)
{
    if (tree == NULL || pattern == NULL || out_matching == NULL) return false;

    Matching *matchings;
    size_t num_matchings = get_all_matchings(tree, pattern, &matchings);

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
Summary: Looks for matching in tree, i.e. tries to construct matching in each node until matching is found (Top-Down)
*/
Node **find_matching(Node **tree, Node *pattern, Matching *out_matching)
{
    if (get_matching(tree, pattern, out_matching)) return tree;
    if (get_type(*tree) == NTYPE_OPERATOR)
    {
        for (size_t i = 0; i < get_num_children(*tree); i++)
        {
            Node **res = find_matching(get_child_addr(*tree, i), pattern, out_matching);
            if (res != NULL) return res;
        }
    }
    return NULL;
}

/*
Summary: Basically the same as find_matching, but discards matching
*/
Node **find_matching_discarded(Node *tree, Node *pattern)
{
    Matching matching;
    return find_matching(&tree, pattern, &matching);
}
