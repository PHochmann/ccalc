#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>

#include "../util/string_util.h"
#include "../util/console_util.h"
#include "../util/alloc_wrappers.h"
#include "../tree/tree_util.h"
#include "../core/evaluation.h"
#include "rewrite_rule.h"
#include "matching.h"

#define MAX_RULESET_ITERATIONS 10000 // To protect against endless loops

/*
Summary: Constructs new rule. Warning: "before" and "after" are not copied, so don't free them!
*/
RewriteRule get_rule(Node *before, Node *after, MappingFilter filter)
{
    preprocess_pattern(before);
    return (RewriteRule){
        .before = before,
        .after  = after,
        .filter = filter
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

static void transform_matched_recursive(Node **parent, Matching *matching)
{
    size_t i = 0;
    while (i < get_num_children(*parent))
    {
        if (get_type(get_child(*parent, i)) == NTYPE_VARIABLE)
        {
            for (size_t j = 0; j < matching->num_mapped; j++)
            {
                if (strcmp(get_var_name(get_child(*parent, i)), matching->mapped_vars[j]) == 0)
                {
                    tree_replace_by_list(parent, i, matching->mapped_nodes[j]);
                    i += matching->mapped_nodes[j].size - 1;
                    break;
                }
            }
            i++;
        }
        else
        {
            if (get_type(get_child(*parent, i)) == NTYPE_OPERATOR)
            {
                transform_matched_recursive(get_child_addr(*parent, i), matching);
            }
            i++;
        }
    }
}

/*
Summary: Substitutes subtree in which matching was found according to rule
*/
void transform_matched(Node *rule_after, Matching *matching, Node **matched_subtree)
{
    if (rule_after == NULL || matching == NULL) return;
    
    Node *transformed = tree_copy(rule_after);

    if (get_type(transformed) == NTYPE_OPERATOR)
    {
        transform_matched_recursive(&transformed, matching);
    }
    else
    {
        if (get_type(transformed) == NTYPE_VARIABLE)
        {
            for (size_t j = 0; j < matching->num_mapped; j++)
            {
                if (strcmp(get_var_name(transformed), matching->mapped_vars[j]) == 0)
                {
                    if (matching->mapped_nodes[j].size != 1) 
                    {
                        software_defect("Trying to replace root with a list > 1.\n");
                    }
                    
                    tree_replace(&transformed, tree_copy(matching->mapped_nodes[j].nodes[0]));
                }
            }
        }
    }
    
    tree_replace(matched_subtree, transformed);
}

/*
Summary: Tries to find matching in tree and directly transforms tree by it
Returns: True when matching could be applied, false otherwise
*/
bool apply_rule(Node **tree, const RewriteRule *rule)
{
    Matching matching;
    // Try to find matching in tree with pattern specified in rule
    Node **res = find_matching(tree, rule->before, &matching, rule->filter);
    if (res == NULL) return false;
    // If matching is found, transform tree with it
    transform_matched(rule->after, &matching, res);

    return true;
}

Vector get_empty_ruleset()
{
    return vec_create(sizeof(RewriteRule), 1);
}

void add_to_ruleset(Vector *rules, RewriteRule rule)
{
    VEC_PUSH_ELEM(rules, RewriteRule, rule);
}

void free_ruleset(Ruleset *rules)
{
    for (size_t i = 0; i < vec_count(rules); i++)
    {
        free_rule(*(RewriteRule*)vec_get(rules, i));
    }
    vec_destroy(rules);
}

size_t apply_ruleset(Node **tree, const Ruleset *ruleset)
{
    VectorIterator iterator = vec_get_iterator(ruleset);
    return apply_ruleset_by_iterator(tree, (Iterator*)&iterator);
}

/*
Summary: Tries to apply rules (priorized by order) until no rule can be applied any more
    Guarantees to terminate after MAX_RULESET_ITERATIONS rule appliances
*/
#include "../tree/tree_to_string.h"
size_t apply_ruleset_by_iterator(Node **tree, Iterator *iterator)
{
    size_t counter = 0;
    while (true)
    {
        bool applied_flag = false;
        RewriteRule *curr_rule = NULL;
        while ((curr_rule = (RewriteRule*)iterator_get_next(iterator)) != NULL)
        {
            if (apply_rule(tree, curr_rule))
            {
                //printf("Applied rule: %s\n", tree_to_str(*tree, true)); // Mem. leak
                applied_flag = true;
                counter++;
                break;
            }
        }
        iterator_reset(iterator);

        if (!applied_flag)
        {
            //printf("End.\n");
            return counter;
        }
        else
        {
            if (counter == MAX_RULESET_ITERATIONS)
            {
                report_error("Aborted due to too many ruleset iterations (max=%d)\n", MAX_RULESET_ITERATIONS);
                print_tree(*tree, true);
                printf("\n");
                return counter;
            }
        }
    }
    return 0; // To make compiler happy
}
