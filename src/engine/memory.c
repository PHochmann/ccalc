#include "memory.h"

/* Summary: Calls free() on each node within tree, including variable's names and constant's values */
void free_tree(Node *tree)
{
    if (tree == NULL) return;
    
    switch (tree->type)
    {
        case NTYPE_OPERATOR:
            for (int i = 0; i < tree->num_children; i++)
            {
                free_tree(tree->children[i]);
            }
            break;
            
        case NTYPE_CONSTANT:
            free(tree->const_value);
            break;
            
        case NTYPE_VARIABLE:
            free(tree->var_name);
            break;
    }
    
    free(tree);
}

void free_matching(Matching matching)
{
    for (int i = 0; i < matching.num_mapped; i++)
    {
        free(matching.mapped_vars[i]);
        free_tree(matching.mapped_nodes[i]);
    }
    free(matching.mapped_vars);
    free(matching.mapped_nodes);
}

void free_rule(RewriteRule rule)
{
    free_tree(rule.before);
    free_tree(rule.after);
}

void free_context(ParsingContext *ctx)
{
    if (ctx == NULL) return;
    free(ctx->operators);
}
