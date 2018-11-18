#include "memory.h"

// Summary: Calls free() on each node within tree, including variable's names, constant's values and operator's children
void free_tree(Node *tree)
{
    if (tree == NULL) return;
    free_tree_preserved(tree);
    free(tree);
}

// Summary: Same as free_tree, but root will be preserved (root's name or value is free'd)
void free_tree_preserved(Node *tree)
{
    if (tree == NULL) return;
    
    switch (tree->type)
    {
        case NTYPE_OPERATOR:
            for (size_t i = 0; i < tree->num_children; i++)
            {
                free_tree(tree->children[i]);
            }
            free(tree->children);
            break;
            
        case NTYPE_CONSTANT:
            free(tree->const_value);
            break;
            
        case NTYPE_VARIABLE:
            free(tree->var_name);
            break;
    }
}

void free_matching(Matching matching)
{
    for (size_t i = 0; i < matching.num_mapped; i++)
    {
        free(matching.mapped_vars[i]);
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
