#include <string.h>
#include "matching.h"
#include "filters.h"

bool prefix_filter(char *var, NodeList nodes)
{
    // Check special rules
    switch (var[0])
    {
        case MATCHING_CONST_PREFIX:
            if (nodes.size != 1 || get_type(nodes.nodes[0]) != NTYPE_CONSTANT)
            {
                return false;
            }
            break;
        case MATCHING_CONST_OR_VAR_PREFIX:
            if (nodes.size != 1 || get_type(nodes.nodes[0]) == NTYPE_OPERATOR)
            {
                return false;
            }
            break;
        case MATCHING_OP_PREFIX:
            if (nodes.size != 1 || get_type(nodes.nodes[0]) != NTYPE_OPERATOR)
            {
                return false;
            }
            break;
        case MATCHING_OP_OR_VAR_PREFIX:
            if (nodes.size != 1 || get_type(nodes.nodes[0]) == NTYPE_CONSTANT)
            {
                return false;
            }
            break;
        case MATCHING_LITERAL_VAR_PREFIX:
            if (nodes.size != 1 || get_type(nodes.nodes[0]) != NTYPE_VARIABLE
                || strcmp(var + 1, get_var_name(nodes.nodes[0])) != 0)
            {
                return false;
            }
    }
    return true;
}
