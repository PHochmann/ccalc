#include <stdio.h>
#include <string.h>

#include "../transformation/matching.h"
#include "../console_util.h"
#include "../tree/tree_to_string.h"
#include "../core/arith_context.h"
#include "cmd_playground.h"

int cmd_playground_check(char *input)
{
    return strcmp("playground", input) == 0;
}

/*
Summary: Opens file and processes its content as from stdin
*/
bool cmd_playground_exec(__attribute__((unused)) char *input, __attribute__((unused)) int code)
{
    char *pattern_str = NULL;
    Node *pattern = NULL;
    char *tree_str = NULL;
    Node *tree = NULL;

    do
    {
        free(pattern_str);
        if (!ask_input(stdin, &pattern_str, "pattern: "))
        {
            goto cleanup;
        }
    } while (parse_input(g_ctx, pattern_str, &pattern) != PERR_SUCCESS && (printf("Syntax Error.\n") || true));

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
        size_t num_matchings = get_all_matchings(&tree, pattern, &matchings);
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
        free(matchings);

        if (num_matchings == 0)
        {
            printf("No matching found.\n");
        }

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
