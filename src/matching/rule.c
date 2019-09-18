#include "rule.h"
#include "matching.h"

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

void free_rule(RewriteRule rule)
{
    free_tree(rule.before);
    free_tree(rule.after);
}

/*
Summary: Tries to find matching in tree and directly transforms tree by it
Returns: True when matching could be applied, false otherwise
*/
bool apply_rule(Node *tree, RewriteRule *rule)
{
    Matching matching;
    // Try to find matching in tree with pattern specified in rule
    if (!find_matching(rule->context, tree, rule->before, &matching)) return false;
    // If matching is found, transform tree with it
    transform_matched_by_rule(rule->context, rule->after, &matching);
    free_matching(matching);
    return true;
}

// All following functions are experimental!

// Does not change any rules, automatically mallocs new rule's before and after (untested!)
bool infer_rule(ParsingContext *ctx, RewriteRule *first, RewriteRule *second, RewriteRule *out_rule)
{
    Matching conversion;
    if (!get_matching(ctx, second->before, first->after, &conversion)) return false;

    // We don't want to transform the same node, but a fresh copy
    conversion.matched_tree = malloc(sizeof(Node));

    // Transform first rule's left side by matching. This is the new rule's left side.
    transform_matched_by_rule(ctx, first->before, &conversion);
    *out_rule = get_rule(ctx, conversion.matched_tree, malloc(sizeof(Node)));
    // The new rule's right side is the same as the second rule's left side. Copy it.
    *out_rule->after = tree_copy(ctx, second->after);

    return true;
}

/*
Summary: Tries to apply rules in round-robin fashion until no rule can be applied any more
Params
    max_iterations: maximal number of times the set is iterated, 0 for no cap (this makes non-termination possible)
Returns: Number of successful appliances
*/
size_t apply_ruleset(Node *tree, size_t num_rules, RewriteRule *rules, size_t max_iterations)
{
    size_t i = 0;
    int res = 0;
    
    while (i < max_iterations || max_iterations == 0)
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
