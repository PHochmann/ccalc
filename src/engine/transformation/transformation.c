#include <stdlib.h>
#include <string.h>
#include "../util/console_util.h"
#include "../tree/tree_util.h"
#include "transformation.h"

static void transform_matched_recursive(const Pattern *pattern, const Matching *matching, Node **parent)
{
    size_t i = 0;
    while (i < get_num_children(*parent))
    {
        if (get_type(get_child(*parent, i)) == NTYPE_VARIABLE)
        {
            for (size_t j = 0; j < pattern->num_free_vars; j++)
            {
                if (strcmp(get_var_name(get_child(*parent, i)), pattern->free_vars[j]) == 0)
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
                transform_matched_recursive(pattern, matching, get_child_addr(*parent, i));
            }
            i++;
        }
    }
}

/*
Summary: Substitutes subtree in which matching was found according to rule
*/
void transform_by_matching(const Pattern *pattern, const Matching *matching, Node *to_transform)
{
    if (to_transform == NULL || matching == NULL) return;

    if (get_type(to_transform) == NTYPE_OPERATOR)
    {
        transform_matched_recursive(pattern, matching, &to_transform);
    }
    else
    {
        if (get_type(to_transform) == NTYPE_VARIABLE)
        {
            for (size_t j = 0; j < pattern->num_free_vars; j++)
            {
                if (strcmp(get_var_name(to_transform), pattern->free_vars[j]) == 0)
                {
                    if (matching->mapped_nodes[j].size != 1) 
                    {
                        software_defect("Trying to replace root with a list > 1.\n");
                    }
                    tree_replace(&to_transform, tree_copy(matching->mapped_nodes[j].nodes[0]));
                    break;
                }
            }
        }
    }
}
