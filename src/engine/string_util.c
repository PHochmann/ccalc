#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "string_util.h"

#define OP_COLOR    "\x1B[47m\x1B[22;30m" // White background, black foreground
#define CONST_COLOR "\x1B[1;33m"          // Yellow
#define VAR_COLOR   "\x1B[1;36m"          // Cyan
#define COL_RESET   "\033[0m"

#define EMPTY_TAB  "    "
#define LINE_TAB   "│   "
#define BRANCH_TAB "├── "
#define END_TAB    "└── "

bool begins_with(char *prefix, char *string)
{
    size_t prefix_length = strlen(prefix);
    size_t string_length = strlen(string);
    if (prefix_length > string_length) return false;
    return strncmp(prefix, string, prefix_length) == 0;
}

void print_constant(ParsingContext *ctx, Node *node)
{
    char value[ctx->recomm_str_len];
    ctx->to_string(node->const_value, ctx->recomm_str_len, value);
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
            for (size_t i = 0; i < node->num_children; i++)
            {
                print_tree_visual_rec(ctx, node->children[i], layer + 1,
                    (i == node->num_children - 1) ? vert_lines : (vert_lines | ((unsigned int)1 << layer)));
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

// Singleton to encapsulate current state
struct PrintingState
{
    ParsingContext *ctx;
    char *buf;
    size_t buf_size;
    bool col;
    size_t num_written;
};

// Updates state after snprintf
void update_state(struct PrintingState *state, int res)
{
    if (res >= 0)
    {
        state->num_written += res;
    }
    else
    {
        return;
    }
    
    state->buf += res; // Advance buffer

    if ((size_t)res <= state->buf_size)
    {
        state->buf_size -= res;
    }
    else
    {
        state->buf_size = 0;
    }
}

// Helper function to print and advance buffer
void to_buf(struct PrintingState *state, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    update_state(state, vsnprintf(state->buf, state->buf_size, format, args));
    va_end(args);
}

void p_open(struct PrintingState *state)
{
    to_buf(state, "(");
}

void p_close(struct PrintingState *state)
{
    to_buf(state, ")");
}

void tree_inline_rec(struct PrintingState *state, Node *node, bool l, bool r)
{
    switch (node->type)
    {
        case NTYPE_CONSTANT:
            if (state->col) to_buf(state, CONST_COLOR);
            update_state(state, state->ctx->to_string(node->const_value, state->buf_size, state->buf));
            if (state->col) to_buf(state, COL_RESET);
            break;
            
        case NTYPE_VARIABLE:
            to_buf(state, state->col ? VAR_COLOR "%s" COL_RESET : "%s", node->var_name);
            break;
            
        case NTYPE_OPERATOR:
            switch (node->op->placement)
            {
                case OP_PLACE_PREFIX:
                    // Constants do not need to be left-protected
                    if (node->op->arity == 0) l = false;

                    if (l) p_open(state);
                    to_buf(state, node->op->name);
                    
                    if (node->op->arity != 0)
                    {
                        if (node->children[0]->type == NTYPE_OPERATOR
                            && node->children[0]->op->precedence <= node->op->precedence)
                        {
                            p_open(state);
                            tree_inline_rec(state, node->children[0], false, false);
                            p_close(state);
                        }
                        else
                        {
                            tree_inline_rec(state, node->children[0], true, !l && r);
                        }
                    }
                    
                    if (l) p_close(state);
                    break;
                    
                case OP_PLACE_POSTFIX:
                    if (r) p_open(state);
                    
                    if (node->op->arity != 0)
                    {
                        if (node->children[0]->type == NTYPE_OPERATOR
                            && node->children[0]->op->precedence < node->op->precedence)
                        {
                            p_open(state);
                            tree_inline_rec(state, node->children[0], false, false);
                            p_close(state);
                        }
                        else
                        {
                            tree_inline_rec(state, node->children[0], l && !r, true);
                        }
                    }
                    
                    to_buf(state, "%s", node->op->name);
                    if (r) p_close(state);
                    break;
                    
                case OP_PLACE_FUNCTION:
                    to_buf(state, "%s(", node->op->name);
                    for (size_t i = 0; i < node->num_children; i++)
                    {
                        tree_inline_rec(state, node->children[i], false, false);
                        if (i < node->num_children - 1) to_buf(state, ", ");
                    }
                    p_close(state);
                    break;
                    
                case OP_PLACE_INFIX:
                {
                    Node *childA = node->children[0];
                    Node *childB = node->children[1];

                    if (childA->type == NTYPE_OPERATOR
                        && (childA->op->precedence < node->op->precedence
                            || (childA->op->precedence == node->op->precedence
                                && (node->op->assoc == OP_ASSOC_RIGHT
                                    || (node->op->assoc == OP_ASSOC_BOTH
                                        && STANDARD_ASSOC == OP_ASSOC_RIGHT
                                        && node->op != childA->op)))))
                    {
                        p_open(state);
                        tree_inline_rec(state, childA, false, false);
                        p_close(state);
                    }
                    else
                    {
                        tree_inline_rec(state, childA, l, true);
                    }

                    if (strlen(node->op->name) == 1)
                    {
                        to_buf(state, "%s", node->op->name);
                    }
                    else
                    {
                        to_buf(state, " %s ", node->op->name);
                    }
                    
                    if (childB->type == NTYPE_OPERATOR
                        && (childB->op->precedence < node->op->precedence
                            || (childB->op->precedence == node->op->precedence
                                && (node->op->assoc == OP_ASSOC_LEFT
                                    || (node->op->assoc == OP_ASSOC_BOTH
                                        && STANDARD_ASSOC == OP_ASSOC_LEFT
                                        && node->op != childB->op)))))
                    {
                        p_open(state);
                        tree_inline_rec(state, childB, false, false);
                        p_close(state);
                    }
                    else
                    {
                        tree_inline_rec(state, childB, true, r);
                    }
                }
            }
        }
}

// Summary: Fills buffer with representation of tree
// Returns: Length of output, even if buffer was not sufficient (without \0)
size_t tree_inline(ParsingContext *context, Node *node, char *buffer, size_t buffer_size, bool color)
{
    // In case nothing is printed, we still want to have a proper string
    if (buffer_size != 0) *buffer = '\0';

    struct PrintingState state = {
        .ctx = context,
        .buf = buffer,
        .buf_size = buffer_size,
        .col = color,
        .num_written = 0
    };

    tree_inline_rec(&state, node, false, false);
    return state.num_written;
}
