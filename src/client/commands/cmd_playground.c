#include <stdio.h>
#include <string.h>

#include "../../engine/transformation/matching.h"
#include "../../engine/transformation/rewrite_rule.h"
#include "../../engine/transformation/transformation.h"
#include "../../engine/transformation/rule_parsing.h"
#include "../../engine/util/console_util.h"
#include "../../engine/util/string_util.h"
#include "../../engine/tree/tree_util.h"
#include "../../engine/tree/tree_to_string.h"
#include "../core/arith_context.h"
#include "../simplification/propositional_context.h"
#include "../simplification/propositional_evaluation.h"
#include "cmd_playground.h"

#define SIMPLIFY_COMMAND   "simplify "
#define VISUALIZE_COMMAND  "visualize "
#define PLAYGROUND_COMMAND "playground"
#define SIMPLIFY_CODE   1
#define PLAYGROUND_CODE 2
#define VISUALIZE_CODE  3
#define MATCHINGS_PRINT_THRESHOLD 10

int cmd_playground_check(const char *input)
{
    if (begins_with(SIMPLIFY_COMMAND, input)) return SIMPLIFY_CODE;
    if (begins_with(VISUALIZE_COMMAND, input)) return VISUALIZE_CODE;
    if (strcmp(input, PLAYGROUND_COMMAND) == 0) return PLAYGROUND_CODE;
    return 0;
}

bool cmd_playground_exec(char *input, int code)
{
    if (code == SIMPLIFY_CODE || code == VISUALIZE_CODE)
    {
        input += strlen(SIMPLIFY_COMMAND);
        Node *node;
        if (!arith_parse_and_postprocess(input, "Error: %s\n", &node)) return false;
        if (code == VISUALIZE_CODE)
        {
            print_tree_visually(node);
        }

        print_tree(node, true);
        printf("\n");

        free_tree(node);
        return true;
    }
    else
    {
        char *rule_str = NULL;
        RewriteRule rule;
        char *tree_str = NULL;
        Node *tree = NULL;

        while (true)
        {
            if (!ask_input(stdin, &rule_str, "rule: "))
            {
                goto cleanup;
            }

            if (!parse_rule(rule_str, g_ctx, g_propositional_ctx, &rule))
            {
                free(rule_str);
                rule_str = NULL;
                continue;
            }
            break;
        }

        while (true)
        {
            if (!ask_input(stdin, &tree_str, "tree: "))
            {
                break;
            }

            if (parse_input(g_ctx, tree_str, &tree) != PERR_SUCCESS)
            {
                printf("Syntax error.\n");
                goto loop_cleanup;
            }

            Matching matching;
            if (get_matching((const Node**)&tree, &rule.pattern, propositional_checker, &matching))
            {
                Node *transformed = tree_copy(rule.after);
                transform_by_matching(rule.pattern.num_free_vars, rule.pattern.free_vars, &matching, &transformed);
                print_tree(transformed, true);
                printf("\n");
                free_tree(transformed);
            }
            else
            {
                printf("No matching found.\n");
            }

            loop_cleanup:
            free(tree_str);
            free_tree(tree);
            tree_str = NULL;
            tree = NULL;
        }

        free_rule(rule);

        cleanup:
        free(rule_str);
        free(tree_str);
        free_tree(tree);
        printf("\n");
        return true;
    }
}
