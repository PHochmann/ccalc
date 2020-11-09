#include <string.h>

#include "matching.h"
#include "filters.h"
#include "evaluation.h"
#include "../tree/tree_util.h"

bool prefix_filter(const char *var, NodeList nodes, __attribute__((unused)) Matching *m)
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

bool exponent_even_filter(const char *var, NodeList nodes, __attribute__((unused)) Matching *m)
{
    if (strcmp(var, "y") == 0)
    {
        // The following should not be needed due to the nature of the matching algorithm
        // Be brave and dereference!
        //if (nodes.size != 1) return false;

        if (count_all_variable_nodes(nodes.nodes[0]) == 0)
        {
            double res = arith_evaluate(nodes.nodes[0]);
            if (res != (int)res) return false;
            if ((int)res % 2 == 0)
            {
                return true;
            }
        }
        return false;
    }
    else
    {
        return true;
    }
}

// deriv(x,y) -> 0
bool constant_derivative_filter(const char *var, NodeList nodes, Matching *m)
{
    if (strcmp(var, "y") == 0)
    {
        if (get_variable_nodes(&m->mapped_nodes[0].nodes[0], get_var_name(nodes.nodes[0]), NULL) == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return true;
    }
}
