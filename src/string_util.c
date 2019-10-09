#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "string_util.h"

#define CONSTANT_TYPE_FMT "%-.30g"

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

void print_tree_visual_rec(Node *node, unsigned char layer, unsigned int vert_lines)
{
    if (layer != 0)
    {
        for (unsigned char i = 0; i < layer - 1; i++)
        {
            printf(vert_lines & ((unsigned int)1 << i) ? LINE_TAB : EMPTY_TAB);
        }
        printf(vert_lines & ((unsigned int)1 << (layer - 1)) ? BRANCH_TAB : END_TAB);
    }

    switch (get_type(node))
    {
        case NTYPE_OPERATOR:
            printf(OP_COLOR "%s" COL_RESET "\n", get_op(node)->name);
            for (size_t i = 0; i < get_num_children(node); i++)
            {
                print_tree_visual_rec(get_child(node, i), layer + 1,
                    (i == get_num_children(node) - 1) ? vert_lines : (vert_lines | ((unsigned int)1 << layer)));
            }
            break;
            
        case NTYPE_CONSTANT:
            printf(CONST_COLOR CONSTANT_TYPE_FMT COL_RESET "\n", get_const_value(node));
            break;
            
        case NTYPE_VARIABLE:
            printf(VAR_COLOR "%s" COL_RESET "\n", get_var_name(node));
            break;
    }
}

/*
Summary: Draws coloured tree to stdout
*/
void print_tree_visual(Node *node)
{
    if (node == NULL) return;
    print_tree_visual_rec(node, 0, 0);
}

// Algorithm to print tree visually ends here. What follows is an algorithm to stringify a tree into a single line.

// Singleton to encapsulate current state
struct PrintingState
{
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
void to_buffer(struct PrintingState *state, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    update_state(state, vsnprintf(state->buf, state->buf_size, format, args));
    va_end(args);
}

// Helper functions
void p_open(struct PrintingState *state)
{
    to_buffer(state, "(");
}

void p_close(struct PrintingState *state)
{
    to_buffer(state, ")");
}

void tree_inline_rec(struct PrintingState *state, Node *node, bool l, bool r);

void inline_prefix(struct PrintingState *state, Node *node, bool l, bool r)
{
    if (l) p_open(state);
    to_buffer(state, get_op(node)->name);
    
    if (get_type(get_child(node, 0)) == NTYPE_OPERATOR
        && get_op(get_child(node, 0))->precedence <= get_op(node)->precedence)
    {
        p_open(state);
        tree_inline_rec(state, get_child(node, 0), false, false);
        p_close(state);
    }
    else
    {
        // Subexpression needs to be right-protected when expression of 'node' is not encapsulated in parentheses
        // (!l, Otherwise redundant parentheses would be printed) and itself needs to be right-protected
        tree_inline_rec(state, get_child(node, 0), true, !l && r);
    }
    
    if (l) p_close(state);
}

void inline_postfix(struct PrintingState *state, Node *node, bool l, bool r)
{
    if (r) p_open(state);

    // It should be safe to dereference first child
    if (get_type(get_child(node, 0)) == NTYPE_OPERATOR
        && get_op(get_child(node, 0))->precedence < get_op(node)->precedence)
    {
        p_open(state);
        tree_inline_rec(state, get_child(node, 0), false, false);
        p_close(state);
    }
    else
    {
        // See analog case of infix operator for conditions for left-protection
        tree_inline_rec(state, get_child(node, 0), l && !r, true);
    }
    
    to_buffer(state, "%s", get_op(node)->name);
    if (r) p_close(state);
}

void inline_function(struct PrintingState *state, Node *node)
{
    if (get_op(node)->arity != 0)
    {
        to_buffer(state, "%s(", get_op(node)->name);
        for (size_t i = 0; i < get_num_children(node); i++)
        {
            tree_inline_rec(state, get_child(node, i), false, false);
            if (i < get_num_children(node) - 1) to_buffer(state, ", ");
        }
        p_close(state);
    }
    else
    {
        to_buffer(state, "%s", get_op(node)->name);
    }
}

void inline_infix(struct PrintingState *state, Node *node, bool l, bool r)
{
    Node *childA = get_child(node, 0);
    Node *childB = get_child(node, 1);

    // Checks if left operand of infix operator which itself is an operator needs to be wrapped in parentheses
    // This is the case when:
    //    - It has a lower precedence
    //    - It has the same precedence but associates to the right
    //      (Same precedence -> same associativity, see consistency rules for operator set in context.c)
    if (get_type(childA) == NTYPE_OPERATOR
        && (get_op(childA)->precedence < get_op(node)->precedence
            || (get_op(childA)->precedence == get_op(node)->precedence
                && get_op(node)->assoc == OP_ASSOC_RIGHT)))
    {
        p_open(state);
        tree_inline_rec(state, childA, false, false);
        p_close(state);
    }
    else
    {
        tree_inline_rec(state, childA, l, true);
    }

    to_buffer(state, strlen(get_op(node)->name) == 1 ? "%s" : " %s ", get_op(node)->name);
    
    // Checks if right operand of infix operator needs to be wrapped in parentheses (see analog case for left operand)
    if (get_type(childB) == NTYPE_OPERATOR
        && (get_op(childB)->precedence < get_op(node)->precedence
            || (get_op(childB)->precedence == get_op(node)->precedence
                && get_op(node)->assoc == OP_ASSOC_LEFT)))
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

/*
Params
    l, r: Indicates whether subexpression represented by 'node' needs to be protected to the left or right.
        It needs to be protected when it is adjacent to an operator on this side.
        When the subexpression starts (ends) with an operator and needs to be protected to the left (right), a parenthesis is printed in between.
*/
void tree_inline_rec(struct PrintingState *state, Node *node, bool l, bool r)
{
    switch (get_type(node))
    {
        case NTYPE_CONSTANT:
            to_buffer(state, state->col ? CONST_COLOR CONSTANT_TYPE_FMT COL_RESET : CONSTANT_TYPE_FMT, get_const_value(node));
            break;
            
        case NTYPE_VARIABLE:
            to_buffer(state, state->col ? VAR_COLOR "%s" COL_RESET : "%s", get_var_name(node));
            break;
            
        case NTYPE_OPERATOR:
            switch (get_op(node)->placement)
            {
                case OP_PLACE_PREFIX:
                    inline_prefix(state, node, l, r);
                    break;
                case OP_PLACE_POSTFIX:
                    inline_postfix(state, node, l, r);
                    break;
                case OP_PLACE_FUNCTION:
                    inline_function(state, node);
                    break;
                case OP_PLACE_INFIX:
                    inline_infix(state, node, l, r);
                    break;
            }
        }
}

// Summary: Fills buffer with representation of tree
// Returns: Length of output, even if buffer was not sufficient (without \0)
size_t tree_inline(Node *node, char *buffer, size_t buffer_size, bool color)
{
    // In case nothing is printed, we still want to have a proper string
    if (buffer_size != 0) *buffer = '\0';

    struct PrintingState state = {
        .buf = buffer,
        .buf_size = buffer_size,
        .col = color,
        .num_written = 0
    };

    tree_inline_rec(&state, node, false, false);
    return state.num_written;
}
