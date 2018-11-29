#include <stdbool.h>
#include <string.h>

#include "memory.h"
#include "rule.h"
#include "constants.h" 
#include "node.h"
#include "console_util.h"

/*
Summary: Constructs new rule. Warning: "before" and "after" are not copied, so don't free them!
*/
RewriteRule get_rule(ParsingContext *ctx, Node *before, Node *after)
{
    return (RewriteRule){
        .context = ctx,
        .before = before,
        .after = after
    };
}

/*
Summary: Tries to match 'tree' against 'pattern' (only in root)
Returns: true, if matching is found, false if NULL-pointers given in arguments or no matching found
*/
bool get_matching(ParsingContext *ctx, Node *tree, Node *pattern, Matching *out_matching)
{
    if (ctx == NULL || tree == NULL || pattern == NULL || out_matching == NULL) return false;
    
    char *mapped_vars[MAX_VAR_COUNT];
    Node *mapped_nodes[MAX_VAR_COUNT];
    size_t num_mapped_vars = 0;
    
    Node *tree_stack[MAX_STACK_SIZE];
    Node *pattern_stack[MAX_STACK_SIZE];
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
            // 1. Check if variable is bound, if it is, check it. Otherwise, bind.
            case NTYPE_VARIABLE:
                for (size_t i = 0; i < num_mapped_vars; i++)
                {
                    if (strcmp(mapped_vars[i], curr_pattern_n->var_name) == 0) // Already bound
                    {
                        found = true;
                        // Already bound variable not compatible with this occurence
                        if (!tree_equals(ctx, mapped_nodes[i], curr_tree_n)) return false;
                    }
                }
                
                if (!found)
                {
                    // Check special rules
                    if (begins_with(CONST_PREFIX, curr_pattern_n->var_name) && curr_tree_n->type != NTYPE_CONSTANT) return false;
                    if (begins_with(VAR_PREFIX, curr_pattern_n->var_name) && curr_tree_n->type != NTYPE_VARIABLE) return false;
                    if (begins_with(LITERAL_PREFIX, curr_pattern_n->var_name))
                    {
                        if (curr_tree_n->type != NTYPE_VARIABLE) return false;
                        if (strcmp(curr_pattern_n->var_name + strlen(LITERAL_PREFIX), curr_tree_n->var_name) != 0) return false;
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

void transform_by_rule(RewriteRule *rule, Matching *matching)
{
    if (rule == NULL || matching == NULL) return;
    
    // We need to save all variable instances in "after" before we substitute because variable names are not sanitized
    Node transformed = tree_copy(rule->context, rule->after);
    Node *var_instances[matching->num_mapped][MAX_VAR_COUNT];
    size_t num_instances[matching->num_mapped];

    for (size_t i = 0; i < matching->num_mapped; i++)
    {
        num_instances[i] = tree_get_variable_instances(&transformed, matching->mapped_vars[i], var_instances[i]);
    }
    
    for (size_t i = 0; i < matching->num_mapped; i++)
    {
        for (size_t j = 0; j < num_instances[i]; j++)
        {
            tree_replace(var_instances[i][j], tree_copy(rule->context, matching->mapped_nodes[i]));
        }
    }
    
    tree_replace(matching->matched_tree, transformed);
}

bool apply_rule(Node *tree, RewriteRule *rule)
{
    Matching matching;
    // Try to find matching in tree with pattern specified in rule
    if (!find_matching(rule->context, tree, rule->before, &matching)) return false;
    // If matching is found, transform tree with it
    transform_by_rule(rule, &matching);
    free_matching(matching);
    return true;
}

/*
Summary: Tries to apply rules in round-robin fashion until no rule can be applied any more
Params
    max_iterations: maximal number of times the set is iterated, -1 for no cap (this makes non-termination possible)
Returns: Number of successful appliances
*/
int apply_ruleset(Node *tree, RewriteRule *rules, size_t num_rules, int max_iterations)
{
    int i = 0;
    int res = 0;
    
    while (i < max_iterations || max_iterations == -1)
    {
        bool applied_flag = false;
        
        for (size_t j = 0; j < num_rules; j++)
        {
            if (apply_rule(tree, &rules[j]))
            {
                res++;
                applied_flag = true;
            }
        }
        
        i++;
        if (!applied_flag) return res;
    }
    
    return res;
}
