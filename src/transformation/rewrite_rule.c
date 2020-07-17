#include "../tree/tree_util.h"
#include "../core/evaluation.h"
#include "rewrite_rule.h"
#include "matching.h"

#define NEW_VAR_LENGTH 5
#define NORMAL_VAR_ID  0
#define RULE_VAR_ID    1

void mark_vars(Node *tree, char id)
{
    char *vars[MAX_MAPPED_VARS];
    size_t num_vars = list_variables(tree, vars);
    size_t safe_var_count = count_variables(tree, false);
    for (size_t i = 0; i < num_vars; i++)
    {
        Node **nodes[safe_var_count];
        size_t num_nodes = get_variable_nodes(&tree, vars[i], nodes);
        for (size_t j = 0; j < num_nodes; j++)
        {
            set_id(*(nodes[j]), id);
        }
    }
}

/*
Summary: Constructs new rule. Warning: "before" and "after" are not copied, so don't free them!
*/
RewriteRule get_rule(Node **before, Node **after)
{
    mark_vars(*before, RULE_VAR_ID);
    mark_vars(*after, RULE_VAR_ID);
    return (RewriteRule){
        .before = *before,
        .after  = *after
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
    
    Node *transformed = tree_copy(rule_after);
    for (size_t i = 0; i < matching->num_mapped; i++)
    {
        replace_variable_nodes_by_list(&transformed, matching->mapped_nodes[i], matching->mapped_vars[i], RULE_VAR_ID);
    }
    mark_vars(transformed, NORMAL_VAR_ID);
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
#include "../tree/tree_to_string.h"
#include <stdio.h>
void apply_ruleset(Node **tree, size_t num_rules, RewriteRule *ruleset)
{
    while (true)
    {
        bool applied_flag = false;
        for (size_t j = 0; j < num_rules; j++)
        {
            if (apply_rule(tree, &ruleset[j]))
            {
                #ifdef DEBUG
                printf("[%zu] ", j);
                print_tree(*tree, true);
                printf("\n");
                #endif
                applied_flag = true;
                break;
            }
        }
        if (!applied_flag)
        {
            #ifdef DEBUG
            printf("End.\n");
            #endif
            return;
        }
    }
}
