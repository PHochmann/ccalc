#include <stdlib.h>
#include <string.h>
#include "../util/console_util.h"
#include "../tree/tree_util.h"
#include "transformation.h"

static void transform_matched_recursive(const Matching *matching, Node **parent)
{
    for (ssize_t i = 0; i < (ssize_t)get_num_children(*parent); i++)
    {
        Node *child = get_child(*parent, i);
        if (get_type(child) == NTYPE_VARIABLE)
        {
            size_t id = get_id(child);
            tree_replace_by_list(parent, i, matching->mapped_nodes[id]);
            i += matching->mapped_nodes[id].size - 1;
        }
        else
        {
            if (get_type(child) == NTYPE_OPERATOR)
            {
                transform_matched_recursive(matching, get_child_addr(*parent, i));
            }
        }
    }
}

/*
Summary: Substitutes subtree in which matching was found according to rule
*/
void transform_by_matching(const Matching *matching, Node **to_transform)
{
    if (to_transform == NULL || matching == NULL) return;

    if (get_type(*to_transform) == NTYPE_OPERATOR)
    {
        transform_matched_recursive(matching, to_transform);
    }
    else
    {
        if (get_type(*to_transform) == NTYPE_VARIABLE)
        {
            if (matching->mapped_nodes[get_id(*to_transform)].size != 1) 
            {
                software_defect("Trying to replace root with a list != 1.\n");
            }
            tree_replace(to_transform, tree_copy(matching->mapped_nodes[get_id(*to_transform)].nodes[0]));
        }
    }
}
