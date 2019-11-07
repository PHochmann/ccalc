#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include "tree_to_string.h"
#include "../parsing/tokenizer.h"

#define EMPTY_TAB  "    "
#define LINE_TAB   "│   "
#define BRANCH_TAB "├── "
#define END_TAB    "└── "

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

// Helper function to write to buffer and advance it
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

void tree_to_string_internal(struct PrintingState *state, Node *node, bool l, bool r);

void prefix_to_string(struct PrintingState *state, Node *node, bool l, bool r)
{
    if (l) p_open(state);
    to_buffer(state, get_op(node)->name);
    
    if (get_type(get_child(node, 0)) == NTYPE_OPERATOR
        && get_op(get_child(node, 0))->precedence <= get_op(node)->precedence)
    {
        p_open(state);
        tree_to_string_internal(state, get_child(node, 0), false, false);
        p_close(state);
    }
    else
    {
        // Subexpression needs to be right-protected when expression of 'node' is not encapsulated in parentheses
        // (!l, otherwise redundant parentheses would be printed) and itself needs to be right-protected
        tree_to_string_internal(state, get_child(node, 0), true, !l && r);
    }
    
    if (l) p_close(state);
}

void postfix_to_string(struct PrintingState *state, Node *node, bool l, bool r)
{
    if (r) p_open(state);

    // It should be safe to dereference first child
    if (get_type(get_child(node, 0)) == NTYPE_OPERATOR
        && get_op(get_child(node, 0))->precedence < get_op(node)->precedence)
    {
        p_open(state);
        tree_to_string_internal(state, get_child(node, 0), false, false);
        p_close(state);
    }
    else
    {
        // See analog case of infix operator for conditions for left-protection
        tree_to_string_internal(state, get_child(node, 0), l && !r, true);
    }
    
    to_buffer(state, "%s", get_op(node)->name);
    if (r) p_close(state);
}

void function_to_string(struct PrintingState *state, Node *node)
{
    if (get_op(node)->arity != 0)
    {
        to_buffer(state, "%s(", get_op(node)->name);
        for (size_t i = 0; i < get_num_children(node); i++)
        {
            tree_to_string_internal(state, get_child(node, i), false, false);
            if (i < get_num_children(node) - 1) to_buffer(state, ",");
        }
        p_close(state);
    }
    else
    {
        to_buffer(state, "%s", get_op(node)->name);
    }
}

void infix_to_string(struct PrintingState *state, Node *node, bool l, bool r)
{
    Node *childL = get_child(node, 0);
    Node *childR = get_child(node, 1);

    // Checks if left operand of infix operator which itself is an operator needs to be wrapped in parentheses
    // This is the case when:
    //    - It has a lower precedence
    //    - It has the same precedence but associates to the right
    //      (Same precedence -> same associativity, see consistency rules for operator set in context.c)
    if (get_type(childL) == NTYPE_OPERATOR
        && (get_op(childL)->precedence < get_op(node)->precedence
            || (get_op(childL)->precedence == get_op(node)->precedence
                && get_op(node)->assoc == OP_ASSOC_RIGHT)))
    {
        p_open(state);
        tree_to_string_internal(state, childL, false, false);
        p_close(state);
    }
    else
    {
        tree_to_string_internal(state, childL, l, true);
    }

    to_buffer(state, is_letter(get_op(node)->name[0]) ? " %s " : "%s", get_op(node)->name);
    
    // Checks if right operand of infix operator needs to be wrapped in parentheses (see analog case for left operand)
    if (get_type(childR) == NTYPE_OPERATOR
        && (get_op(childR)->precedence < get_op(node)->precedence
            || (get_op(childR)->precedence == get_op(node)->precedence
                && get_op(node)->assoc == OP_ASSOC_LEFT)))
    {
        p_open(state);
        tree_to_string_internal(state, childR, false, false);
        p_close(state);
    }
    else
    {
        tree_to_string_internal(state, childR, true, r);
    }
}

/*
Params
    l, r: Indicates whether subexpression represented by 'node' needs to be protected to the left or right.
        It needs to be protected when it is adjacent to an operator on this side.
        When the subexpression starts (ends) with an operator and needs to be protected to the left (right), a parenthesis is printed in between.
*/
void tree_to_string_internal(struct PrintingState *state, Node *node, bool l, bool r)
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
                    prefix_to_string(state, node, l, r);
                    break;
                case OP_PLACE_POSTFIX:
                    postfix_to_string(state, node, l, r);
                    break;
                case OP_PLACE_FUNCTION:
                    function_to_string(state, node);
                    break;
                case OP_PLACE_INFIX:
                    infix_to_string(state, node, l, r);
                    break;
            }
        }
}

// Summary: Fills buffer with representation of tree
// Returns: Length of output, even if buffer was not sufficient (without \0)
size_t tree_to_string(Node *node, char *buffer, size_t buffer_size, bool color)
{
    // In case nothing is printed, we still want to have a proper string
    if (buffer_size != 0) *buffer = '\0';

    struct PrintingState state = {
        .buf = buffer,
        .buf_size = buffer_size,
        .col = color,
        .num_written = 0
    };

    tree_to_string_internal(&state, node, false, false);
    return state.num_written;
}

void unsafe_tree_to_string(Node *node, char *buffer, bool color)
{
    tree_to_string(node, buffer, SIZE_MAX, color);
}

/*
Returns: Number of bytes needed to store string representation of node (including \0)
*/
size_t sizeof_tree_to_string(Node *node, bool color)
{
    return tree_to_string(node, NULL, 0, color) + 1;
}

/*
Summary: Prints result of tree_to_string to stdout
*/
void print_tree(Node *node, bool color)
{
    char buffer[sizeof_tree_to_string(node, color)];
    unsafe_tree_to_string(node, buffer, color);
    printf("%s", buffer);
}

/* Algorithm to put tree into single string ends here. What follows is a method to visually print a tree-like representation for debugging purposes */

void print_tree_visually_internal(Node *node, unsigned char layer, unsigned int vert_lines)
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
                print_tree_visually_internal(get_child(node, i), layer + 1,
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
Summary: Draws colored tree to stdout
*/
void print_tree_visually(Node *node)
{
    if (node == NULL) return;
    print_tree_visually_internal(node, 0, 0);
}
