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

#define ARROW           " -> "
#define COMMENT_PREFIX  '\''

#define MAX_RULESET_ITERATIONS 1000 // To protect against endless loops

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

void transform_matched_recursive(Node **parent, Matching *matching)
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
                        report_error("Software defect: trying to replace root with a list > 1.\n");
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
bool apply_rule(Node **tree, RewriteRule *rule)
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

void free_ruleset(Vector *rules)
{
    for (size_t i = 0; i < vec_count(rules); i++)
    {
        free_rule(VEC_GET_ELEM(rules, RewriteRule, i));
    }
    vec_destroy(rules);
}

/*
Summary: Tries to apply rules (priorized by order) until no rule can be applied any more
    Guarantees to terminate after MAX_RULESET_ITERATIONS rule appliances
*/
//#include "../tree/tree_to_string.h"
size_t apply_ruleset(Node **tree, Ruleset *rules)
{
    size_t counter = 0;
    while (true)
    {
        bool applied_flag = false;
        for (size_t j = 0; j < vec_count(rules); j++)
        {
            if (apply_rule(tree, (RewriteRule*)vec_get(rules, j)))
            {
                //printf("Applied rule %zu: %s\n", j, tree_to_str(*tree, true));
                applied_flag = true;
                counter++;
                break;
            }
        }

        if (!applied_flag || counter == MAX_RULESET_ITERATIONS)
        {
            //printf("End.\n");
            return counter;
        }
    }
    return 0; // To make compiler happy
}

// Same as apply_rulset, but rules are within a LinkedList instead of a Vector
// Todo: Can this be refactored?
size_t apply_rule_list(Node **tree, LinkedList *rules)
{
    size_t counter = 0;
    while (true)
    {
        bool applied_flag = false;
        ListNode *curr = rules->first;
        while (curr != NULL)
        {
            if (apply_rule(tree, (RewriteRule*)curr->data))
            {
                applied_flag = true;
                counter++;
                break;
            }
            curr = curr->next;
        }
        if (!applied_flag || counter == MAX_RULESET_ITERATIONS)
        {
            return counter;
        }
    }
    return 0; // To make compiler happy
}

bool parse_rule(char *string, ParsingContext *ctx, MappingFilter default_filter, Vector *out_ruleset)
{
    if (string[0] == COMMENT_PREFIX || string[0] == '\0')
    {
        return true;
    }

    char *right = strstr(string, ARROW);
    if (right == NULL)
    {
        return false;
    }

    right[0] = '\0';
    right += strlen(ARROW);
    Node *left_n = parse_conveniently(ctx, string);
    if (left_n == NULL)
    {
        return false;
    }

    Node *right_n = parse_conveniently(ctx, right);
    if (right_n == NULL)
    {
        free_tree(left_n);
        return false;
    }

    add_to_ruleset(out_ruleset, get_rule(left_n, right_n, default_filter));
    return true;
}

bool parse_ruleset_from_string(char *string, ParsingContext *ctx, MappingFilter default_filter, Vector *out_ruleset)
{
    // String is likely to be readonly - copy it
    char *copy = malloc_wrapper(strlen(string) + 1);
    strcpy(copy, string);

    *out_ruleset = get_empty_ruleset();
    size_t line_no = 0;
    char *line = copy;
    while (line != NULL)
    {
        line_no++;
        char *next_line = strstr(line, "\n");
        if (next_line != NULL)
        {
            next_line[0] = '\0';
        }

        if (!parse_rule(line, ctx, default_filter, out_ruleset))
        {
            report_error("Failed parsing ruleset in line %zu.\n", line_no);
            goto error;
        }

        line = next_line;
        if (line != NULL) line++; // Skip newline char
    }

    vec_trim(out_ruleset);
    free(copy);
    return true;

    error:
    free(copy);
    free_ruleset(out_ruleset);
    return false;
}
