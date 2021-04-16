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
bool get_rule(Pattern pattern, Node *after, RewriteRule *out_rule)
{
    // If variables in 'after' a present, check if all of them occur in 'before'
    // If not, a syntax error is present in rule
    size_t num_var_nodes = count_all_variable_nodes(after);
    if (num_var_nodes > 0)
    {
        const char *after_vars[MAX_MAPPED_VARS];
        bool sufficient = false;
        size_t num_vars_distinct = list_variables(after, MAX_MAPPED_VARS, after_vars, &sufficient);

        if (!sufficient)
        {
            return false;
        }

        for (size_t i = 0; i < num_vars_distinct; i++)
        {
            if (get_variable_nodes((const Node**)&pattern.pattern, after_vars[i], 0, NULL) == 0)
            {
                return false;
            }
        }
    }

    const char *free_vars[MAX_MAPPED_VARS];
    size_t num_free_vars = list_variables(pattern.pattern, MAX_MAPPED_VARS, free_vars, NULL);
    tree_copy_IDs(after, num_free_vars, free_vars);

    *out_rule = (RewriteRule){
        .pattern = pattern,
        .after  = after,
    };
    return true;
}

/*
Summary: Frees trees "before" and "after"
*/
void free_rule(RewriteRule *rule)
{
    free_pattern(&(rule->pattern));
    free_tree(rule->after);
}

// Needed to preserve error information
void set_tok_index_for_all(Node *tree, size_t index)
{
    set_token_index(tree, index);
    if (get_type(tree) == NTYPE_OPERATOR)
    {
        for (size_t i = 0; i < get_num_children(tree); i++)
        {
            set_tok_index_for_all(get_child(tree, i), index);
        }
    }
}

/*
Summary: Tries to find matching in tree and directly transforms tree by it
Returns: True when matching could be applied, false otherwise
Params
    eval: Is allowed to be NULL
*/
bool apply_rule(Node **tree, const RewriteRule *rule, ConstraintChecker checker)
{
    Matching matching;
    // Try to find matching in tree with pattern specified in rule
    Node **matched_subtree = find_matching((const Node**)tree, &rule->pattern, checker, &matching);
    if (matched_subtree == NULL) return false;
    // If matching is found, transform tree with it
    Node *transformed = tree_copy(rule->after);
    // Every new node in rhs of rule emerged from root of matched subtree
    set_tok_index_for_all(transformed, get_token_index(*matched_subtree));
    transform_by_matching(&matching, &transformed);
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
        free_rule((RewriteRule*)vec_get(rules, i));
    }
    vec_destroy(rules);
}

size_t apply_ruleset(Node **tree, const Vector *ruleset, ConstraintChecker checker, size_t cap)
{
    VectorIterator iterator = vec_get_iterator(ruleset);
    return apply_ruleset_by_iterator(tree, (Iterator*)&iterator, checker, cap);
}

/*
Summary: Tries to apply rules (priorized by order) until no rule can be applied any more
    Guarantees to terminate after MAX_RULESET_ITERATIONS rule appliances
*/
size_t apply_ruleset_by_iterator(Node **tree, Iterator *iterator, ConstraintChecker checker, size_t cap)
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
            if (apply_rule(tree, curr_rule, checker))
            {
                #ifdef DEBUG
                printf("Applied rule ");
                print_tree(curr_rule->pattern.pattern, true);
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
            return counter;
        }
        else
        {
            if (counter == cap)
            {
                return counter;
            }
        }
    }
    return 0; // To make compiler happy
}
