#include <stdlib.h>
#include "transformation.h"

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
void transform_by_matching(Node *to_transform, Matching *matching)
{
    if (to_transform == NULL || matching == NULL) return;

    if (get_type(to_transform) == NTYPE_OPERATOR)
    {
        transform_matched_recursive(&to_transform, matching);
    }
    else
    {
        if (get_type(to_transform) == NTYPE_VARIABLE)
        {
            for (size_t j = 0; j < matching->num_mapped; j++)
            {
                if (strcmp(get_var_name(to_transform), matching->mapped_vars[j]) == 0)
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
