#include <stdbool.h>
#include <string.h>

#include "matching.h"
#include "../parsing/node.h"
#include "../util/string_util.h" // For begins_with

#define MAX_STACK_SIZE 20

/*
Summary: Tries to match "tree" against "pattern" (only in root)
Returns: True, if matching is found, false if NULL-pointers given in arguments or no matching found
*/
bool get_matching(Node **tree, Node *pattern, Matching *out_matching)
{
    if (tree == NULL || *tree == NULL || pattern == NULL || out_matching == NULL) return false;
    
    size_t total_mapped_vars = count_variables_distinct(pattern);
    char *mapped_vars[total_mapped_vars];
    Node *mapped_nodes[total_mapped_vars];
    size_t num_mapped_vars = 0;
    
    Node *tree_stack[MAX_STACK_SIZE];
    Node *pattern_stack[MAX_STACK_SIZE];
    size_t num_stack = 0;
    
    tree_stack[0] = *tree;
    pattern_stack[0] = pattern;
    num_stack = 1;
    
    while (num_stack != 0)
    {
        Node *curr_pattern = pattern_stack[num_stack - 1];
        Node *curr_tree = tree_stack[num_stack - 1];
        num_stack--;
        
        switch (get_type(curr_pattern))
        {
            // 1. Check if variable is bound, if it is, check occurrence. Otherwise, bind.
            case NTYPE_VARIABLE:
            {
                bool already_bound = false;
                for (size_t i = 0; i < num_mapped_vars; i++)
                {
                    if (strcmp(mapped_vars[i], get_var_name(curr_pattern)) == 0) // Already bound
                    {
                        // Is already bound variable equal to this occurrence?
                        // If not, fail here
                        if (!tree_equals(mapped_nodes[i], curr_tree))
                        {
                            return false;
                        }

                        already_bound = true;
                        break;
                    }
                }
                
                if (!already_bound)
                {
                    // Check special rules
                    if (begins_with(CONST_PREFIX, get_var_name(curr_pattern))
                        && get_type(curr_tree) != NTYPE_CONSTANT)
                    {
                        return false;
                    }

                    if (begins_with(VAR_PREFIX, get_var_name(curr_pattern))
                        && get_type(curr_tree) != NTYPE_VARIABLE)
                    {
                        return false;
                    }

                    if (begins_with(NAME_PREFIX, get_var_name(curr_pattern))
                        && (get_type(curr_tree) != NTYPE_VARIABLE
                            || strcmp(get_var_name(curr_pattern) + strlen(NAME_PREFIX),
                                get_var_name(curr_tree)) != 0))
                    {
                        return false;
                    }
                    
                    // Bind variable
                    mapped_vars[num_mapped_vars] = get_var_name(curr_pattern);
                    mapped_nodes[num_mapped_vars] = curr_tree;
                    num_mapped_vars++;
                }
                break;
            }
                
            // 2. Check constants for equality
            case NTYPE_CONSTANT:
                if (!tree_equals(curr_pattern, curr_tree))
                {
                    return false;
                }
                break;
                
            // 3. Check operator and arity for equality
            case NTYPE_OPERATOR:
                if (get_type(curr_tree) != NTYPE_OPERATOR
                    || get_op(curr_pattern) != get_op(curr_tree)
                    || get_num_children(curr_pattern) != get_num_children(curr_tree))
                {
                    return false;
                }

                for (size_t i = 0; i < get_num_children(curr_pattern); i++)
                {
                    tree_stack[num_stack + i] = get_child(curr_tree, i);
                    pattern_stack[num_stack + i] = get_child(curr_pattern, i);
                }

                num_stack += get_num_children(curr_pattern);
                break;
        }
    }
    
    // We successfully found matching! Construct it:
    out_matching->matched_tree = tree;            // Used to replace later
    out_matching->num_mapped   = num_mapped_vars; // Should be the same as total_mapped_vars
    out_matching->mapped_vars  = malloc(sizeof(char*) * num_mapped_vars);
    out_matching->mapped_nodes = malloc(sizeof(Node*) * num_mapped_vars);
    
    for (size_t i = 0; i < num_mapped_vars; i++)
    {
        out_matching->mapped_vars[i] = mapped_vars[i];
        out_matching->mapped_nodes[i] = mapped_nodes[i];
    }
    
    return true;
}

/*
Summary: Frees everything except matched_tree
*/
void free_matching(Matching matching)
{
    free(matching.mapped_vars);
    free(matching.mapped_nodes);
}

/*
Summary: Looks for matching in tree, i.e. tries to construct matching in each node until matching is found (Top-Down)
*/
bool find_matching(Node **tree, Node *pattern, Matching *out_matching)
{
    if (get_matching(tree, pattern, out_matching)) return true;
    
    if (get_type(*tree) == NTYPE_OPERATOR)
    {
        for (size_t i = 0; i < get_num_children(*tree); i++)
        {
            if (find_matching(get_child_addr(*tree, i), pattern, out_matching)) return true;
        }
    }
    
    return false;
}

/*
Summary: Basically the same as find_matching, but discards matching
*/
bool find_matching_discarded(Node *tree, Node *pattern)
{
    Matching matching;
    if (find_matching(&tree, pattern, &matching))
    {
        free_matching(matching);
        return true;
    }
    return false;
}
