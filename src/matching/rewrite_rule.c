#include "rewrite_rule.h"
#include "matching.h"

/*
Summary: Constructs new rule. Warning: "before" and "after" are not copied, so don't free them!
*/
RewriteRule get_rule(Node *before, Node *after)
{
    return (RewriteRule){
        .before = before,
        .after = after
    };
}

void free_rule(RewriteRule rule)
{
    free_tree(rule.before);
    free_tree(rule.after);
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

/*
Summary: Tries to find matching in tree and directly transforms tree by it
Returns: True when matching could be applied, false otherwise
*/
bool apply_rule(ParsingContext *ctx, Node *tree, RewriteRule *rule)
{
    Matching matching;
    // Try to find matching in tree with pattern specified in rule
    if (!find_matching(ctx, tree, rule->before, &matching)) return false;
    // If matching is found, transform tree with it
    transform_matched_by_rule(ctx, rule->after, &matching);
    free_matching(matching);
    return true;
}
