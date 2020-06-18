#include <stdbool.h>
#include <string.h>

#include "matching.h"
#include "../tree/tree_util.h"

/*
Code in this file lacks buffer-overflow protection
Todo: Add protection
*/

#define MAX_MATCHINGS 20
#define MAX_PARTITIONS 20

bool nodelists_equal(NodeList *a, NodeList *b)
{
    if (a->size != b->size) return false;
    for (size_t i = 0; i < a->size; i++)
    {
        if (tree_compare(a->nodes[i], b->nodes[i]) != NULL) return false;
    }
    return true;
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
    Matching *out_matchings)
{
    size_t candidates[MAX_PARTITIONS][num_pattern_children];
    size_t results[MAX_PARTITIONS][num_pattern_children];
    size_t sizes[MAX_PARTITIONS];
    size_t sums[MAX_PARTITIONS];
    size_t num_candidates = 1;
    size_t num_results = 0;

    sizes[0] = 0;
    sums[0] = 0;

    while (num_candidates != 0)
    {
        num_candidates--;
        size_t i = num_candidates;

        if (sizes[i] == num_pattern_children && sums[i] == num_tree_children)
        {
            // Candidate is finished and can be put into result list
            memcpy(results[num_results++], candidates[i], sizeof(size_t) * num_pattern_children);
            continue;
        }
        
        if (sizes[i] < num_pattern_children)
        {
            if (get_type(pattern_children[sizes[i]]) == NTYPE_VARIABLE
                && get_var_name(pattern_children[sizes[i]])[0] == MATCHING_LIST_PREFIX)
            {
                NodeList *list = lookup_mapped_var(&matching, get_var_name(pattern_children[sizes[i]]));
                if (list != NULL)
                {
                    // List is already bound, thus also its length is bound
                    // Reactivate candidate with updated length
                    num_candidates++;
                    candidates[i][sizes[i]] = list->size;
                    sums[i] += list->size;
                    sizes[i] += 1;
                }
                else
                {
                    size_t sum_temp = sums[i];
                    size_t size_temp = sizes[i];

                    // List is not bound, we need add a candidate for every possible list length
                    for (size_t j = 0; j + sum_temp <= num_tree_children; j++)
                    {
                        num_candidates++;

                        // We can copy over prefix because it is not overwritten
                        // But don't copy to the same destination (memcpy pointers are restricted)
                        if (j != 0)
                        {
                            memcpy(candidates[i + j], candidates[i], sizeof(size_t) * size_temp);
                        }

                        candidates[i + j][size_temp] = j;
                        sums[i + j] = sum_temp + j;
                        sizes[i + j] = size_temp + 1;
                    }
                }
            }
            else
            {
                // Any non-list node in pattern corresponds to exactly one node in tree
                if (sums[i] < num_tree_children)
                {
                    num_candidates++;
                    candidates[i][sizes[i]] = 1;
                    sums[i] += 1;
                    sizes[i] += 1;
                }
            }
        }
    }

    /*printf("NUM_RESULTS: %zu\n", num_results);
    for (size_t x = 0; x < num_results; x++)
    {
        printf("[%zu] ", x);
        for (size_t y = 0; y < num_pattern_children; y++)
        {
            printf("%zu ", results[x][y]);
        }
        printf("\n");
    }*/

    // We successfully found all possible partitions of the parameter list of tree on all parameters in pattern
    // Now, try to match them
    Matching *additional_buffer = malloc(MAX_MATCHINGS * sizeof(Matching));
    while (num_results != 0)
    {
        num_results--;
        Matching *matchingsA = additional_buffer;
        Matching *matchingsB = out_matchings;
        size_t num_matchings = 1;
        matchingsA[0] = matching;
        size_t curr_child_start = 0;

        // Try to match iteratively with all previously found matchings
        for (size_t i = 0; i < num_pattern_children; i++)
        {
            size_t num_new_matchings = 0;
            for (size_t j = 0; j < num_matchings; j++)
            {
                num_new_matchings += extend_matching(matchingsA[j], pattern_children[i],
                    (NodeList){ .nodes = tree_children + curr_child_start, .size = results[num_results][i] },
                    matchingsB + num_new_matchings);
            }

            // Swap matchingsA and matchingsB to use new matchings as starting point in next iteration
            curr_child_start += results[num_results][i];
            Matching *temp = matchingsB;
            matchingsB = matchingsA;
            matchingsA = temp;
            num_matchings = num_new_matchings;
        }

        // Partition works when there are matchings in the end
        if (num_matchings != 0)
        {
            // Stop after first working partition
            // Copy results of last iteration to output-buffer if last buffer wasn't already output-buffer
            if (out_matchings != matchingsA)
            {
                memcpy(out_matchings, matchingsA, sizeof(Matching) * num_matchings);
            }
            free(additional_buffer);
            return num_matchings;
        }
    }

    // No matchings found
    free(additional_buffer);
    return 0;
}

size_t extend_matching(Matching matching,
    Node *pattern,
    NodeList tree_list,
    Matching *out_matchings)
{
    /*printf("extend_matching: pattern=");
    print_tree(pattern, false);
    printf(" tree list: ");
    for (size_t i = 0; i < tree_list.size; i++)
    {
        print_tree(tree_list.nodes[i], false);
        printf(" | ");
    }
    printf("\n");*/

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
                // Bind variable
                out_matchings[0] = matching;
                out_matchings[0].mapped_vars[matching.num_mapped]  = get_var_name(pattern);
                out_matchings[0].mapped_nodes[matching.num_mapped] = tree_list;
                out_matchings[0].mapped_ids[matching.num_mapped]   = get_id(pattern); // This is a dirty trick to avoid sanitizing variable names
                out_matchings[0].num_mapped++;
                return 1;
            }
        }
            
        // 2. Check constants for equality
        case NTYPE_CONSTANT:
        {
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
            if (get_type(tree_list.nodes[0]) != NTYPE_OPERATOR
                || get_op(pattern) != get_op(tree_list.nodes[0])
                /*|| get_num_children(curr_pattern) != get_num_children(curr_tree)*/)
            {
                return 0;
            }

            return match_parameter_lists(matching, get_num_children(pattern), get_child_addr(pattern, 0),
                get_num_children(tree_list.nodes[0]), get_child_addr(tree_list.nodes[0], 0), out_matchings);
        }
    }
    return 0;
}

/*
Summary: Tries to match "tree" against "pattern" (only in root)
Returns: True, if matching is found, false if NULL-pointers given in arguments or no matching found
*/
bool get_matching(Node *tree, Node *pattern, Matching *out_matching)
{
    if (tree == NULL || pattern == NULL || out_matching == NULL) return false;

    // Due to exponential many partitions, a lot of states can occur. Use heap.
    Matching *matchings = malloc(MAX_MATCHINGS * sizeof(Matching));
    size_t num_matchings = extend_matching((Matching){ .num_mapped = 0 }, pattern, (NodeList){ .size = 1, .nodes = &tree }, matchings);

    // Stack should contain only one element.
    if (num_matchings > 0)
    {
        *out_matching = matchings[0];
        free(matchings);

        /*printf("~ ~ Matching found ~ ~ num_mapped=%zu\n", out_matching->num_mapped);
        for (size_t i = 0; i < out_matching->num_mapped; i++)
        {
            printf("%s -> ", out_matching->mapped_vars[i]);
            for (size_t j = 0; j < out_matching->mapped_nodes[i].size; j++)
            {
                printf("%zu: ", j);
                print_tree(out_matching->mapped_nodes[i].nodes[j], false);
                printf(", ");
            }
            printf("\n");
        }*/

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
    if (get_matching(*tree, pattern, out_matching)) return tree;
    
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
