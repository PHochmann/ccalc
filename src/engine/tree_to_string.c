#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "tree_to_string.h"

#define EMPTY_TAB  "    "
#define LINE_TAB   "│   "
#define BRANCH_TAB "├── "
#define END_TAB    "└── "

void print_constant(ParsingContext *ctx, Node *node)
{
    char value[ctx->min_str_len];
    ctx->to_string(node->const_value, value, ctx->min_str_len);
    printf(CONST_COLOR "%s" COL_RESET, value);
}

void print_variable(Node *node)
{
    printf(VAR_COLOR "%s" COL_RESET, node->var_name);
}

void print_tree_visual_rec(ParsingContext *ctx, Node *node, unsigned char layer, unsigned int vert_lines)
{
    if (layer != 0)
    {
        for (unsigned char i = 0; i < layer - 1; i++)
        {
            printf(vert_lines & ((unsigned int)1 << i) ? LINE_TAB : EMPTY_TAB);
        }
        printf(vert_lines & ((unsigned int)1 << (layer - 1)) ? BRANCH_TAB : END_TAB);
    }

    switch (node->type)
    {
        case NTYPE_OPERATOR:
            printf(OP_COLOR "%s" COL_RESET "\n", node->op->name);
            for (Arity i = 0; i < node->num_children; i++)
            {
                print_tree_visual_rec(ctx, node->children[i], layer + 1,
                    (i == (Arity)(node->num_children - 1)) ? vert_lines : (vert_lines | ((unsigned int)1 << layer)));
            }
            break;
            
        case NTYPE_CONSTANT:
            print_constant(ctx, node);
            printf("\n");
            break;
            
        case NTYPE_VARIABLE:
            print_variable(node);
            printf("\n");
            break;
    }
}

/*
Summary: Draws coloured tree to stdout
*/
void print_tree_visual(ParsingContext *ctx, Node *node)
{
    if (ctx == NULL || node == NULL) return;
    print_tree_visual_rec(ctx, node, 0, 0);
}

void print_buffered(char *string, char **buffer, ssize_t *buffer_size)
{
    while (*string != '\0')
    {
        if (*buffer_size > 0)
        {
            **buffer = *string;
            (*buffer)++;
        }
        (*buffer_size)--;
        string++;
    }
}

/*
Summary: Same as print_buffered, but doesn't fragment string (useful for ANSI color codes)
*/
void print_buffered_protected(char *string, char **buffer, ssize_t *buffer_size)
{
    if (*buffer_size < (ssize_t)strlen(string)) return;
    print_buffered(string, buffer, buffer_size);
}

void tree_inline_rec(ParsingContext *ctx, Node *node, char **buffer, ssize_t *buffer_size, bool colours, bool l, bool r)
{
    char constant_value[ctx->min_str_len];

    switch (node->type)
    {
        case NTYPE_CONSTANT:
            if (colours) print_buffered_protected(CONST_COLOR, buffer, buffer_size);
            ctx->to_string(node->const_value, constant_value, ctx->min_str_len);
            print_buffered(constant_value, buffer, buffer_size);
            if (colours) print_buffered_protected(COL_RESET, buffer, buffer_size);
            break;
            
        case NTYPE_VARIABLE:
            if (colours) print_buffered_protected(VAR_COLOR, buffer, buffer_size);
            print_buffered(node->var_name, buffer, buffer_size);
            if (colours) print_buffered_protected(COL_RESET, buffer, buffer_size);
            break;
            
        case NTYPE_OPERATOR:
            switch (node->op->placement)
            {
                case OP_PLACE_PREFIX:
                    if (node->op->arity == 0) l = false;

                    if (l) print_buffered("(", buffer, buffer_size);
                    print_buffered(node->op->name, buffer, buffer_size);
                    
                    if (node->op->arity != 0)
                    {
                        if (node->children[0]->type == NTYPE_OPERATOR
                            && node->children[0]->op->precedence <= node->op->precedence)
                        {
                            print_buffered("(", buffer, buffer_size);
                            tree_inline_rec(ctx, node->children[0], buffer, buffer_size, colours, false, false);
                            print_buffered(")", buffer, buffer_size);
                        }
                        else
                        {
                            tree_inline_rec(ctx, node->children[0], buffer, buffer_size, colours, true, !l && r);
                        }
                    }
                    
                    if (l) print_buffered(")", buffer, buffer_size);
                    break;
                    
                case OP_PLACE_POSTFIX:
                    if (r) print_buffered("(", buffer, buffer_size);
                    
                    if (node->op->arity != 0)
                    {
                        if (node->children[0]->type == NTYPE_OPERATOR
                            && node->children[0]->op->precedence < node->op->precedence)
                        {
                            print_buffered("(", buffer, buffer_size);
                            tree_inline_rec(ctx, node->children[0], buffer, buffer_size, colours, false, false);
                            print_buffered(")", buffer, buffer_size);
                        }
                        else
                        {
                            tree_inline_rec(ctx, node->children[0], buffer, buffer_size, colours, l && !r, true);
                        }
                    }
                    
                    print_buffered(node->op->name, buffer, buffer_size);
                    if (r) print_buffered(")", buffer, buffer_size);
                    break;
                    
                case OP_PLACE_FUNCTION:
                    print_buffered(node->op->name, buffer, buffer_size);
                    print_buffered("(", buffer, buffer_size);
                    for (Arity i = 0; i < node->num_children; i++)
                    {
                        tree_inline_rec(ctx, node->children[i], buffer, buffer_size, colours, false, false);
                        if (i != (Arity)(node->num_children - 1)) print_buffered(", ", buffer, buffer_size);
                    }
                    print_buffered(")", buffer, buffer_size);
                    break;
                    
                case OP_PLACE_INFIX:
                    if (node->children[0]->type == NTYPE_OPERATOR
                        && (node->children[0]->op->precedence < node->op->precedence
                            || (node->children[0]->op->precedence == node->op->precedence
                                && node->op->assoc == OP_ASSOC_RIGHT)))
                    {
                        print_buffered("(", buffer, buffer_size);
                        tree_inline_rec(ctx, node->children[0], buffer, buffer_size, colours, false, false);
                        print_buffered(")", buffer, buffer_size);
                    }
                    else
                    {
                        tree_inline_rec(ctx, node->children[0], buffer, buffer_size, colours, l, true);
                    }

                    if (strlen(node->op->name) == 1)
                    {
                        print_buffered(node->op->name, buffer, buffer_size);
                    }
                    else
                    {
                        print_buffered(" ", buffer, buffer_size);
                        print_buffered(node->op->name, buffer, buffer_size);
                        print_buffered(" ", buffer, buffer_size);
                    }
                    
                    if (node->children[1]->type == NTYPE_OPERATOR
                        && (node->children[1]->op->precedence < node->op->precedence
                            || (node->children[1]->op->precedence == node->op->precedence
                                && node->op->assoc == OP_ASSOC_LEFT)))
                    {
                        print_buffered("(", buffer, buffer_size);
                        tree_inline_rec(ctx, node->children[1], buffer, buffer_size, colours, false, false);
                        print_buffered(")", buffer, buffer_size);
                    }
                    else
                    {
                        tree_inline_rec(ctx, node->children[1], buffer, buffer_size, colours, true, r);
                    }
            }
            
            break;
    }
}

/*
Summary: Fills buffer with representation of tree
Returns: Total length of representation, even if buffer was too small
*/
size_t tree_inline(ParsingContext *ctx, Node *node, char *buffer, size_t buffer_size, bool colours)
{
    if (ctx == NULL || node == NULL) return 0;

    ssize_t buffer_size_copy = buffer_size - 1;
    tree_inline_rec(ctx, node, &buffer, &buffer_size_copy, colours, false, false);
    if (buffer_size != 0) *buffer = '\0';
    return buffer_size - buffer_size_copy - 1;
}
