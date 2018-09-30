#include "memory.h"

/* Summary: Calls free() on each node within tree, including variable's names and constant's values */
void free_tree(Node *tree, bool preserve_root)
{
    if (tree == NULL) return;
    
    switch (tree->type)
    {
        case NTYPE_OPERATOR:
            for (int i = 0; i < tree->num_children; i++)
            {
                free_tree(tree->children[i], false);
            }
            break;
            
        case NTYPE_CONSTANT:
            free(tree->const_value);
            break;
            
        case NTYPE_VARIABLE:
            free(tree->var_name);
            break;
    }
    
    if (!preserve_root) free(tree);
}

void free_matching(Matching matching)
{
    for (int i = 0; i < matching.num_mapped; i++)
    {
        free(matching.mapped_vars[i]);
    }
    free(matching.mapped_vars);
    free(matching.mapped_nodes);
}

void free_rule(RewriteRule rule)
{
    free_tree(rule.before, false);
    free_tree(rule.after, false);
}

void free_context(ParsingContext *ctx)
{
    if (ctx == NULL) return;
    free(ctx->operators);
}
