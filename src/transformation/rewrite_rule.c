#include "rewrite_rule.h"
#include "matching.h"

#include "../tree/tree_to_string.h"
#include <stdio.h>

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

/*
Summary: Frees trees "before" and "after"
*/
void free_rule(RewriteRule rule)
{
    free_tree(rule.before);
    free_tree(rule.after);
}

/*
Summary: Substitutes subtree in which matching was found according to rule
*/
void transform_matched_by_rule(Node *rule_after, Matching *matching, Node **matched_subtree)
{
    if (rule_after == NULL || matching == NULL) return;
    
    /*
     * TODO: Variable names should be sanitized
     */

    Node *transformed = tree_copy(rule_after);
    for (size_t i = 0; i < matching->num_mapped; i++)
    {
        replace_variable_nodes_by_list(&transformed, matching->mapped_nodes[i], matching->mapped_vars[i]);
    }
    
    tree_replace(matched_subtree, transformed);
}

/*
Summary: Tries to find matching in tree and directly transforms tree by it
Returns: True when matching could be applied, false otherwise
*/
bool apply_rule(Node **tree, RewriteRule *rule)
{
    Matching matching;
    
    // Try to find matching in tree with pattern specified in rule
    Node **res = find_matching(tree, rule->before, &matching);
    if (res == NULL) return false;

    // If matching is found, transform tree with it
    transform_matched_by_rule(rule->after, &matching, res);

    return true;
}

/*
Summary: Tries to apply rules (priorized by order) until no rule can be applied any more
    Possibly not terminating!
*/
void apply_ruleset(Node **tree, size_t num_rules, RewriteRule *ruleset)
{
    while (true)
    {
        bool applied_flag = false;
        for (size_t j = 0; j < num_rules; j++)
        {
            if (apply_rule(tree, &ruleset[j]))
            {
                applied_flag = true;
                break;
            }
        }
        if (!applied_flag) return;
    }
}
