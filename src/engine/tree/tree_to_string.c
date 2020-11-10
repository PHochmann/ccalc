#include <stdarg.h>
#include <stdio.h>

#include "tree_to_string.h"
#include "../util/string_util.h"

#define STRBUILDER_STARTSIZE 20
#define OPENING_P "("
#define CLOSING_P ")"

static void p_open(StringBuilder *builder)
{
    strbuilder_append(builder, OPENING_P);
}

static void p_close(StringBuilder *builder)
{
    strbuilder_append(builder, CLOSING_P);
}

static void to_str(StringBuilder *builder, bool color, const Node *node, bool l, bool r);

static void prefix_to_str(StringBuilder *builder, bool color, const Node *node, bool l, bool r)
{
    if (l) p_open(builder);
    strbuilder_append(builder, get_op(node)->name);
    
    if (get_type(get_child(node, 0)) == NTYPE_OPERATOR
        && get_op(get_child(node, 0))->precedence <= get_op(node)->precedence)
    {
        p_open(builder);
        to_str(builder, color, get_child(node, 0), false, false);
        p_close(builder);
    }
    else
    {
        // Subexpression needs to be right-protected when expression of 'node' is not encapsulated in parentheses
        // (!l, otherwise redundant parentheses would be printed) and itself needs to be right-protected
        to_str(builder, color, get_child(node, 0), true, !l && r);
    }
    
    if (l) p_close(builder);
}

static void postfix_to_str(StringBuilder *builder, bool color, const Node *node, bool l, bool r)
{
    if (r) p_open(builder);

    // It should be safe to dereference first child
    if (get_type(get_child(node, 0)) == NTYPE_OPERATOR
        && get_op(get_child(node, 0))->precedence < get_op(node)->precedence)
    {
        p_open(builder);
        to_str(builder, color, get_child(node, 0), false, false);
        p_close(builder);
    }
    else
    {
        // See analog case of infix operator for conditions for left-protection
        to_str(builder, color, get_child(node, 0), l && !r, true);
    }
    
    strbuilder_append(builder, "%s", get_op(node)->name);
    if (r) p_close(builder);
}

static void function_to_str(StringBuilder *builder, bool color, const Node *node)
{
    if (get_op(node)->arity != 0)
    {
        strbuilder_append(builder, "%s(", get_op(node)->name);
        for (size_t i = 0; i < get_num_children(node); i++)
        {
            to_str(builder, color, get_child(node, i), false, false);
            if (i < get_num_children(node) - 1) strbuilder_append(builder, ",");
        }
        p_close(builder);
    }
    else
    {
        strbuilder_append(builder, "%s", get_op(node)->name);
    }
}

static void infix_to_str(StringBuilder *builder, bool color, const Node *node, bool l, bool r)
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
        p_open(builder);
        to_str(builder, color, childL, false, false);
        p_close(builder);
    }
    else
    {
        to_str(builder, color, childL, l, true);
    }

    strbuilder_append(builder, is_letter(get_op(node)->name[0]) ? " %s " : "%s", get_op(node)->name);
    
    // Checks if right operand of infix operator needs to be wrapped in parentheses (see analog case for left operand)
    if (get_type(childR) == NTYPE_OPERATOR
        && (get_op(childR)->precedence < get_op(node)->precedence
            || (get_op(childR)->precedence == get_op(node)->precedence
                && get_op(node)->assoc == OP_ASSOC_LEFT)))
    {
        p_open(builder);
        to_str(builder, color, childR, false, false);
        p_close(builder);
    }
    else
    {
        to_str(builder, color, childR, true, r);
    }
}

/*
Params
    l, r: Indicates whether subexpression represented by 'node' needs to be protected to the left or right.
        It needs to be protected when it is adjacent to an operator on this side.
        When the subexpression starts (ends) with an operator and needs to be protected to the left (right), a parenthesis is printed in between.
*/
static void to_str(StringBuilder *builder, bool color, const Node *node, bool l, bool r)
{
    switch (get_type(node))
    {
        case NTYPE_CONSTANT:
            strbuilder_append(builder, color ? CONST_COLOR CONSTANT_TYPE_FMT COL_RESET : CONSTANT_TYPE_FMT, get_const_value(node));
            break;
            
        case NTYPE_VARIABLE:
            strbuilder_append(builder, color ? VAR_COLOR "%s" COL_RESET : "%s", get_var_name(node));
            break;
            
        case NTYPE_OPERATOR:
            switch (get_op(node)->placement)
            {
                case OP_PLACE_PREFIX:
                    prefix_to_str(builder, color, node, l, r);
                    break;
                case OP_PLACE_POSTFIX:
                    postfix_to_str(builder, color, node, l, r);
                    break;
                case OP_PLACE_FUNCTION:
                    function_to_str(builder, color, node);
                    break;
                case OP_PLACE_INFIX:
                    infix_to_str(builder, color, node, l, r);
                    break;
            }
        }
}

void tree_to_strbuilder(StringBuilder *builder, const Node *node, bool color)
{
    to_str(builder, color, node, false, false);
}

// Summary: Returns string (on heap)
char *tree_to_str(const Node *node, bool color)
{
    StringBuilder builder = strbuilder_create(STRBUILDER_STARTSIZE);
    tree_to_strbuilder(&builder, node, color);
    return strbuilder_to_str(&builder);
}

/*
Summary: Prints result of tree_to_str to stdout
*/
void print_tree(const Node *node, bool color)
{
    char *str = tree_to_str(node, color);
    printf("%s", str);
    free(str);
}

/* Algorithm to put tree into single string ends here.
 * The following function can be used to visually print a tree-like representation for debugging purposes
 */

#define EMPTY_TAB  "    "
#define LINE_TAB   "│   "
#define BRANCH_TAB "├── "
#define END_TAB    "└── "

static void print_tree_visually_internal(const Node *node, unsigned char layer, unsigned int vert_lines)
{
    if (layer != 0)
    {
        for (unsigned char i = 0; i < layer - 1; i++)
        {
            printf((vert_lines & ((unsigned int)1 << i)) ? LINE_TAB : EMPTY_TAB);
        }
        printf((vert_lines & ((unsigned int)1 << (layer - 1))) ? BRANCH_TAB : END_TAB);
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
void print_tree_visually(const Node *node)
{
    if (node == NULL) return;
    print_tree_visually_internal(node, 0, 0);
}
