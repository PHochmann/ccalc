#include <string.h>
#include <stdio.h>

#include "../util/string_util.h"
#include "../util/console_util.h"
#include "../util/alloc_wrappers.h"
#include "../tree/tree_util.h"
#include "../tree/tree_to_string.h"
#include "rewrite_rule.h"
#include "transformation.h"
#include "matching.h"

/*
Summary: Constructs new rule. Warning: "before" and "after" are not copied, so don't free them!
*/
RewriteRule get_rule(Pattern pattern, Node *after)
{
    const char *after_vars[count_all_variable_nodes(after)];
    size_t num_vars_distinct = list_variables(after, SIZE_MAX, after_vars);
    for (size_t i = 0; i < num_vars_distinct; i++)
    {
        if (get_variable_nodes((const Node**)&pattern.pattern, after_vars[i], NULL) == 0)
        {
            print_tree(pattern.pattern, true);
            printf("\n");
            print_tree(after, true);
            printf("\n");
            software_defect("Trying to create a rule that introduces a new variable after appliance.\n");
        }
    }

    return (RewriteRule){
        .pattern = pattern,
        .after  = after,
    };
}

/*
Summary: Frees trees "before" and "after"
*/
void free_rule(RewriteRule rule)
{
    free_pattern(&rule.pattern);
    free_tree(rule.after);
}

/*
Summary: Tries to find matching in tree and directly transforms tree by it
Returns: True when matching could be applied, false otherwise
Params
    eval: Is allowed to be NULL
*/
bool apply_rule(Node **tree, const RewriteRule *rule, Evaluation eval)
{
    Matching matching;
    // Try to find matching in tree with pattern specified in rule
    Node **matched_subtree = find_matching((const Node**)tree, &rule->pattern, eval, &matching);
    if (matched_subtree == NULL) return false;
    // If matching is found, transform tree with it
    Node *transformed = tree_copy(rule->after);
    transform_by_matching(transformed, &matching);
    tree_replace(matched_subtree, transformed);
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

void free_ruleset(Vector *rules)
{
    for (size_t i = 0; i < vec_count(rules); i++)
    {
        free_rule(*(RewriteRule*)vec_get(rules, i));
    }
    vec_destroy(rules);
}

size_t apply_ruleset(Node **tree, const Vector *ruleset, Evaluation eval, size_t cap)
{
    VectorIterator iterator = vec_get_iterator(ruleset);
    return apply_ruleset_by_iterator(tree, (Iterator*)&iterator, eval, cap);
}

/*
Summary: Tries to apply rules (priorized by order) until no rule can be applied any more
    Guarantees to terminate after MAX_RULESET_ITERATIONS rule appliances
*/
size_t apply_ruleset_by_iterator(Node **tree, Iterator *iterator, Evaluation eval, size_t cap)
{
    #ifdef DEBUG
    printf("Starting with: ");
    print_tree(*tree, true);
    printf("\n");
    #endif

    size_t counter = 0;
    while (true)
    {
        bool applied_flag = false;
        RewriteRule *curr_rule = NULL;
        while ((curr_rule = (RewriteRule*)iterator_get_next(iterator)) != NULL)
        {
            if (apply_rule(tree, curr_rule, eval))
            {
                #ifdef DEBUG
                printf("Applied rule ");
                print_tree(curr_rule->before, true);
                printf(" : ");
                print_tree(*tree, true);
                printf("\n");
                #endif
                applied_flag = true;
                counter++;
                break;
            }
        }
        iterator_reset(iterator);
        if (!applied_flag)
        {
            #ifdef DEBUG
            printf("Can't apply any rule. End.\n");
            #endif
            return counter;
        }
        else
        {
            if (counter == cap)
            {
                #ifdef DEBUG
                printf("Iteration cap reached. End.\n");
                #endif
                return counter;
            }
        }
    }
    return 0; // To make compiler happy
}
