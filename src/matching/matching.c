#include <stdbool.h>
#include <string.h>

#include "matching.h"
#include "../parsing/node.h"
#include "../string_util.h" // For begins_with

#define VAR_PREFIX "var_"
#define CONST_PREFIX "const_"
#define NAME_PREFIX "name_"

/*
Summary: Tries to match "tree" against "pattern" (only in root)
Returns: True, if matching is found, false if NULL-pointers given in arguments or no matching found
*/
bool get_matching(ParsingContext *ctx, Node *tree, Node *pattern, Matching *out_matching)
{
    if (ctx == NULL || tree == NULL || pattern == NULL || out_matching == NULL) return false;
    
    char *mapped_vars[MAX_VAR_COUNT];
    Node *mapped_nodes[MAX_VAR_COUNT];
    size_t num_mapped_vars = 0;
    
    Node *tree_stack[MAX_TREE_SEARCH_STACK_SIZE];
    Node *pattern_stack[MAX_TREE_SEARCH_STACK_SIZE];
    size_t num_stack = 0;
    
    tree_stack[0] = tree;
    pattern_stack[0] = pattern;
    num_stack = 1;
    
    while (num_stack != 0)
    {
        Node *curr_pattern_n = pattern_stack[num_stack - 1];
        Node *curr_tree_n = tree_stack[num_stack - 1];
        num_stack--;
        
        bool found = false;
        switch (curr_pattern_n->type)
        {
            // 1. Check if variable is bound, if it is, check occurrence. Otherwise, bind.
            case NTYPE_VARIABLE:
                for (size_t i = 0; i < num_mapped_vars; i++)
                {
                    if (strcmp(mapped_vars[i], curr_pattern_n->var_name) == 0) // Already bound
                    {
                        // Is already bound variable equal to this occurrence?
                        // If not, fail here
                        if (!tree_equals(ctx, mapped_nodes[i], curr_tree_n)) return false;
                        found = true;
                        break;
                    }
                }
                
                if (!found)
                {
                    // Check special rules
                    if (begins_with(CONST_PREFIX, curr_pattern_n->var_name) && curr_tree_n->type != NTYPE_CONSTANT) return false;
                    if (begins_with(VAR_PREFIX, curr_pattern_n->var_name) && curr_tree_n->type != NTYPE_VARIABLE) return false;
                    if (begins_with(NAME_PREFIX, curr_pattern_n->var_name))
                    {
                        if (curr_tree_n->type != NTYPE_VARIABLE) return false;
                        if (strcmp(curr_pattern_n->var_name + strlen(NAME_PREFIX), curr_tree_n->var_name) != 0) return false;
                    }
                    
                    // Bind variable
                    mapped_vars[num_mapped_vars] = curr_pattern_n->var_name;
                    mapped_nodes[num_mapped_vars] = curr_tree_n;
                    num_mapped_vars++;
                }
                
                break;
                
            // 2. Check constants for equality
            case NTYPE_CONSTANT:
                if (!node_equals(ctx, curr_pattern_n, curr_tree_n)) return false;
                break;
                
            // 3. Check operands for equality and add children on stack
            case NTYPE_OPERATOR:
                if (!node_equals(ctx, curr_pattern_n, curr_tree_n)) return false;
                for (size_t i = 0; i < curr_pattern_n->num_children; i++)
                {
                    tree_stack[num_stack + i] = curr_tree_n->children[i];
                    pattern_stack[num_stack + i] = curr_pattern_n->children[i];
                }
                num_stack += curr_pattern_n->num_children;
                break;
        }
    }
    
    // We successfully found matching! Construct it:
    out_matching->matched_tree = tree;
    out_matching->num_mapped = num_mapped_vars;
    out_matching->mapped_vars = malloc(sizeof(char*) * num_mapped_vars);
    out_matching->mapped_nodes = malloc(sizeof(Node*) * num_mapped_vars);
    
    for (size_t i = 0; i < num_mapped_vars; i++)
    {
        out_matching->mapped_vars[i] = malloc(sizeof(char) * (strlen(mapped_vars[i]) + 1));
        strcpy(out_matching->mapped_vars[i], mapped_vars[i]);
        out_matching->mapped_nodes[i] = mapped_nodes[i];
    }
    
    return true;
}

/*
Summary: Frees everything except matched_tree
*/
void free_matching(Matching matching)
{
    for (size_t i = 0; i < matching.num_mapped; i++)
    {
        free(matching.mapped_vars[i]);
    }
    free(matching.mapped_vars);
    free(matching.mapped_nodes);
}

/*
Summary: Looks for matching in tree, i.e. tries to construct matching in each node until matching is found (Top-Down)
*/
bool find_matching(ParsingContext *ctx, Node *tree, Node *pattern, Matching *out_matching)
{
    if (ctx == NULL || tree == NULL || pattern == NULL || out_matching == NULL) return false;
    
    if (get_matching(ctx, tree, pattern, out_matching)) return true;
    
    if (tree->type == NTYPE_OPERATOR)
    {
        for (size_t i = 0; i < tree->num_children; i++)
        {
            if (find_matching(ctx, tree->children[i], pattern, out_matching)) return true;
        }
    }
    
    return false;
}

/*
Summary: Substitutes subtree in which matching was found according to rule
*/
void transform_matched_by_rule(ParsingContext *ctx, Node *rule_after, Matching *matching)
{
    if (ctx == NULL || rule_after == NULL || matching == NULL) return;
    
    // We need to save all variable instances in "after" before we substitute because variable names are not sanitized
    Node transformed = tree_copy(ctx, rule_after);
    Node *var_instances[matching->num_mapped][MAX_VAR_COUNT];
    size_t num_instances[matching->num_mapped];

    for (size_t i = 0; i < matching->num_mapped; i++)
    {
        tree_get_var_instances(&transformed, matching->mapped_vars[i], &num_instances[i], var_instances[i]);
    }
    
    for (size_t i = 0; i < matching->num_mapped; i++)
    {
        for (size_t j = 0; j < num_instances[i]; j++)
        {
            tree_replace(var_instances[i][j], tree_copy(ctx, matching->mapped_nodes[i]));
        }
    }
    
    tree_replace(matching->matched_tree, transformed);
}
