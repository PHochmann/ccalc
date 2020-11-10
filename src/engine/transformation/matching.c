#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "matching.h"
#include "transformation.h"
#include "../util/vector.h"
#include "../tree/tree_util.h"
#include "../util/console_util.h"
#include "../util/string_util.h"

#define VECTOR_STARTSIZE 1

// On heap during matching
typedef struct {
    Pattern *pattern;
    Evaluation eval;
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
            fill_suffix(curr_index,
                pattern_children,
                tree_children,
                &vec_local_matchings,
                &vec_suffixes,
                ctx);

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
            const char *var_name = get_var_name(pattern);

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
                // Check constraints if its okay to bind
                for (size_t i = 0; i < ctx->pattern->num_constraints[id]; i++)
                {
                    Node *cpy_l = tree_copy(ctx->pattern->constraints[id][i].lhs);
                    Node *cpy_r = tree_copy(ctx->pattern->constraints[id][i].rhs);
                    transform_by_matching(cpy_l, &matching);
                    transform_by_matching(cpy_r, &matching);
                    tree_replace_constant_subtrees(&cpy_l, ctx->eval, 0, NULL);
                    tree_replace_constant_subtrees(&cpy_r, ctx->eval, 0, NULL);
                    bool res = tree_equals(cpy_l, cpy_r);
                    free_tree(cpy_l);
                    free_tree(cpy_r);
                    if (!res) return;
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
                && get_op(pattern) == get_op(tree_list.nodes[0]))
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

size_t get_all_matchings(const Node **tree, const Pattern *pattern, Evaluation eval, Matching **out_matchings)
{
    if (tree == NULL || pattern == NULL || out_matchings == NULL) return false;

    // Due to exponentially many partitions, a lot of states can occur. Use heap.
    Vector result = vec_create(sizeof(Matching), VECTOR_STARTSIZE);

    // Create context object and put it on heap, this saves stack space during recursion
    MatchingContext ctx = (MatchingContext){
        .pattern = pattern,
        .eval = eval
    };

    extend_matching(
        &ctx,
        (Matching){ .num_mapped = 0 },
        pattern,
        (NodeList){ .size = 1, .nodes = tree },
        &result);
    *out_matchings = (Matching*)result.buffer;
    return result.elem_count;
}

/*
Summary: suffixess to match "tree" against "pattern" (only in root)
Returns: True, if matching is found, false if NULL-pointers given in arguments or no matching found
*/
bool get_matching(const Node **tree, const Pattern *pattern, Evaluation eval, Matching *out_matching)
{
    if (tree == NULL || pattern == NULL || out_matching == NULL) return false;

    Matching *matchings;
    size_t num_matchings = get_all_matchings(tree, pattern, eval, &matchings);

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
Node **find_matching(const Node **tree, const Pattern *pattern, Evaluation eval, Matching *out_matching)
{
    if (get_matching(tree, pattern, eval, out_matching)) return (Node**)tree;
    if (get_type(*tree) == NTYPE_OPERATOR)
    {
        for (size_t i = 0; i < get_num_children(*tree); i++)
        {
            Node **res = find_matching((const Node**)get_child_addr(*tree, i), pattern, eval, out_matching);
            if (res != NULL) return res;
        }
    }
    return NULL;
}

/*
Summary: Basically the same as find_matching, but discards matching
*/
bool does_match(const Node *tree, const Pattern *pattern, Evaluation eval)
{
    Matching matching;
    if (find_matching((const Node**)&tree, pattern, eval, &matching) != NULL)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/*
Summary: - Sets ids of variable nodes for faster lookup while matching
         - Computes trigger-indices for constraints
*/
Pattern pattern_create(Node *tree, size_t num_constraints, Node **constr_l, Node **constr_r)
{
    Pattern res = (Pattern){
        .pattern         = tree,
        .num_constraints = { 0 }
    };

    size_t num_vars = count_all_variable_nodes(tree);
    const char *vars[num_vars];
    size_t num_vars_distinct = list_variables(tree, num_vars, vars);

    // Step 1: Set id in pattern tree
    size_t curr_id = 0;
    for (size_t i = 0; i < num_vars_distinct; i++)
    {
        if (vars[i][0] != MATCHING_WILDCARD
            && (vars[i][0] != MATCHING_LIST_PREFIX || vars[i][1] != MATCHING_WILDCARD))
        {
            if (curr_id == MAX_MAPPED_VARS)
            {
                software_defect("Trying to preprocess a pattern with too many distinct variables. Increase MAX_MAPPED_VARS.\n");
            }

            Node **nodes[num_vars];
            size_t num_nodes = get_variable_nodes((const Node**)&tree, vars[i], nodes);
            for (size_t j = 0; j < num_nodes; j++)
            {
                set_id(*(nodes[j]), curr_id);
            }
            curr_id++;
        }
    }


    // Step 2: Compute contraints that should be checked when variable with this id is bound
    // This involves computing the variable with the highest id that occurs in the constraint
    for (size_t i = 0; i < num_constraints; i++)
    {
        size_t max_id = 0;
        curr_id = 0;
        for (size_t i = 0; i < num_vars_distinct; i++)
        {
            if (vars[i][0] != MATCHING_WILDCARD
                && (vars[i][0] != MATCHING_LIST_PREFIX || vars[i][1] != MATCHING_WILDCARD))
            {
                if (get_variable_nodes(constr_l + i, vars[curr_id], NULL) != 0
                    || get_variable_nodes(constr_r + i, vars[curr_id], NULL) != 0)
                {
                    max_id = curr_id;
                }
                curr_id++;
            }
        }

        // max_id contains trigger index
        res.constraints[max_id][res.num_constraints[max_id]] = (PatternConstraint){
            .lhs = constr_l[i],
            .rhs = constr_r[i]
        };
        res.num_constraints[max_id]++;
    }

    return res;
}

void free_pattern(Pattern *pattern)
{
    free_tree(pattern->pattern);
    for (size_t i = 0; i < MAX_MAPPED_VARS; i++)
    {
        for (size_t j = 0; j < pattern->num_constraints[i]; j++)
        {
            free_tree(pattern->constraints[i][j].lhs);
            free_tree(pattern->constraints[i][j].rhs);
        }
    }
}
