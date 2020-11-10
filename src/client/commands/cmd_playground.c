#include <stdio.h>
#include <string.h>

#include "../../engine/transformation/matching.h"
#include "../../engine/util/console_util.h"
#include "../../engine/util/string_util.h"
#include "../../engine/tree/tree_to_string.h"
#include "../core/arith_context.h"
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
        char *pattern_str = NULL;
        Node *pattern = NULL;
        char *tree_str = NULL;
        Node *tree = NULL;

        while (true)
        {
            if (!ask_input(stdin, &pattern_str, "pattern: "))
            {
                goto cleanup;
            }

            if (parse_input(g_ctx, pattern_str, &pattern) != PERR_SUCCESS)
            {
                printf("Syntax Error.\n");
                free(pattern_str);
                continue;
            }
            break;
        }

        preprocess_pattern(pattern);

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

            Matching *matchings;
            size_t num_matchings = get_all_matchings((const Node**)&tree, pattern, &matchings, NULL);

            if (num_matchings < MATCHINGS_PRINT_THRESHOLD)
            {
                for (size_t k = 0; k < num_matchings; k++)
                {
                    printf("Matching Nr. %zu:\n", k + 1);
                    for (size_t i = 0; i < matchings[k].num_mapped; i++)
                    {
                        printf("%s -> ", matchings[k].mapped_vars[i]);
                        for (size_t j = 0; j < matchings[k].mapped_nodes[i].size; j++)
                        {
                            print_tree(matchings[k].mapped_nodes[i].nodes[j], true);
                            printf(", ");
                        }
                        printf("\n");
                    }
                }
            }
            else
            {
                printf("%zu matchings found.\n", num_matchings);
            }
            if (num_matchings == 0)
            {
                printf("No matching found.\n");
            }
            free(matchings);

            loop_cleanup:
            free(tree_str);
            free_tree(tree);
            tree_str = NULL;
            tree = NULL;
        }

        cleanup:
        free(pattern_str);
        free_tree(pattern);
        free(tree_str);
        free_tree(tree);
        printf("\n");
        return true;
    }
}
