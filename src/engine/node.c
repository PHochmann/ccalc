#include <string.h>
#include <stdio.h>

#include "node.h"
#include "memory.h"

Node get_node(NodeType type)
{
    Node res;
    res.type = type;
    return res;
}

/* Returns a new node of type NTYPE_VARIABLE and prepares its attributes */
Node get_variable_node(char *var_name)
{
    Node res = get_node(NTYPE_VARIABLE);
    res.var_name = var_name;
    return res;
}

/* Returns a new node of type NTYPE_CONSTANT and prepares its attributes */
Node get_constant_node(void *value)
{
    Node res = get_node(NTYPE_CONSTANT);
    res.const_value = value;
    return res;
}

/* Returns a new node of type NTYPE_OPERATOR and prepares its attributes */
Node get_operator_node(Operator *op)
{
    Node res = get_node(NTYPE_OPERATOR);
    res.op = op;
    res.num_children = 0;
    for (size_t i = 0; i < MAX_CHILDREN; i++) res.children[i] = NULL;
    return res;
}

/* Returns true iff variable node exists in tree
   False indicates save evaluation */
bool tree_contains_variable(Node* tree)
{
    if (tree == NULL) return false;
    
    switch (tree->type)
    {
        case NTYPE_CONSTANT:
            return false;
            
        case NTYPE_VARIABLE:
            return true;
            
        case NTYPE_OPERATOR:
            for (int i = 0; i < tree->num_children; i++)
            {
                if (tree_contains_variable(tree->children[i])) return true;
            }
            return false;
    }
    
    return false;
}

/*
Summary: Lists all variable nodes of given names (e.g. to replace them)
*/
int tree_get_variable_instances(Node *tree, char *variable, Node *out_instances[MAX_VAR_COUNT])
{
    if (tree == NULL) return -1;
    
    int res_count = 0;
    
    Node *node_stack[MAX_STACK_SIZE];
    node_stack[0] = tree;
    int stack_count = 1;
    
    while (stack_count > 0)
    {
        Node *curr_node = node_stack[--stack_count];
        switch (curr_node->type)
        {
            case NTYPE_VARIABLE:
                if (strcmp(curr_node->var_name, variable) == 0)
                {
                    if (res_count == MAX_VAR_COUNT) return -1;
                    out_instances[res_count] = curr_node;
                    res_count++;
                }
                break;
                
            case NTYPE_OPERATOR:
                for (int i = curr_node->num_children - 1; i >= 0; i--)
                {
                    if (stack_count == MAX_STACK_SIZE) return -1;
                    node_stack[stack_count++] = curr_node->children[i];
                }
                break;
                
            case NTYPE_CONSTANT:
                break;
        }
    }
    
    return res_count;
}

/*
Summary: Lists all variable names in tree.
Params
    out_variables must hold at least MAX_VAR_COUNT char-pointers
        (strings are copied, don't forget to call free on each of out_variables' items)
*/
int tree_list_variables(Node *tree, char *out_variables[MAX_VAR_COUNT])
{
    if (tree == NULL) return -1;
    
    int res_count = 0;
    
    Node *node_stack[MAX_STACK_SIZE];
    node_stack[0] = tree;
    int stack_count = 1;
    
    while (stack_count > 0)
    {
        Node *curr_node = node_stack[--stack_count];
        bool flag = false; // To break twice
        
        switch (curr_node->type)
        {
            case NTYPE_VARIABLE:
                for (int i = 0; i < res_count; i++)
                {
                    // Don't add variable if we already found it
                    if (strcmp(out_variables[i], curr_node->var_name) == 0)
                    {
                        flag = true;
                        break;
                    }
                }
                if (flag) break;
                
                // Buffer overflow protection
                if (res_count == MAX_VAR_COUNT) return -1;
                
                out_variables[res_count] = malloc(strlen(curr_node->var_name) + 1);
                strcpy(out_variables[res_count++], curr_node->var_name);
                break;
                
            case NTYPE_OPERATOR:
                for (int i = curr_node->num_children - 1; i >= 0; i--)
                {
                    // Buffer overflow protection
                    if (stack_count == MAX_STACK_SIZE) return -1;
                    
                    node_stack[stack_count++] = curr_node->children[i];
                }
                break;
                
            case NTYPE_CONSTANT:
                break;
        }
    }
    
    return res_count;
}

/*
Summary: Substitutes any occurence of a variable with certain name with a given subtree
Returns: Number of occurences of variable
*/
int tree_substitute_variable(ParsingContext *ctx, Node *tree, Node *tree_to_copy, char *var_name)
{
    if (tree == NULL || tree_to_copy == NULL || var_name == NULL) return 0;
    
    Node *var_instances[MAX_VAR_COUNT];
    int inst_count = tree_get_variable_instances(tree, var_name, var_instances);
    
    for (int i = 0; i < inst_count; i++)
    {
        tree_replace(var_instances[i], tree_copy(ctx, tree_to_copy));
    }
    
    return inst_count;
}

/*
Summary: frees all child-trees and replaces root value-wise
Params
    new_node: Tree 'destination' is replaced with. You may want to copy it before.
*/
void tree_replace(Node *destination, Node new_node)
{
    if (destination == NULL) return;
    free_tree(destination, true);
    *destination = new_node;
}

Node tree_copy(ParsingContext *ctx, Node *tree)
{
    Node res = *tree;
    
    switch (tree->type)
    {
        case NTYPE_OPERATOR:
            for (int i = 0; i < tree->num_children; i++)
            {
                res.children[i] = malloc(sizeof(Node));
                *(res.children[i]) = tree_copy(ctx, tree->children[i]);
            }
            break;
        
        case NTYPE_CONSTANT:
            res.const_value = malloc(ctx->value_size);
            for (size_t i = 0; i < ctx->value_size; i++)
            {
                *((char*)(res.const_value) + i) = *((char*)(tree->const_value) + i);
            }
            break;
            
        case NTYPE_VARIABLE:
            res.var_name = malloc((strlen(tree->var_name) + 1) * sizeof(char));
            strcpy(res.var_name, tree->var_name);
            break;
    }
    
    return res;
}

/*
Summary: Fallback in node_equals that is used when no EqualsHandler is defined in context
*/
bool bytewise_equals(void *a, void *b, size_t value_size)
{
    for (size_t i = 0; i < value_size; i++)
    {
        if (((char*)a)[i] != ((char*)b)[i]) return false;
    }
    
    return true;
}

bool node_equals(ParsingContext *ctx, Node *a, Node *b)
{
    if (ctx == NULL || a == NULL || b == NULL) return false;
    
    if (a->type != b->type) return false;
    
    switch (a->type)
    {
        case NTYPE_OPERATOR:
            if (a->op != b->op) return false;
            if (a->num_children != b->num_children) return false;
            break;
            
        case NTYPE_CONSTANT:
            if (ctx->equals == NULL)
            {
                if (!bytewise_equals(a->const_value, b->const_value, ctx->value_size)) return false;
            }
            else
            {
                if (!ctx->equals(a->const_value, b->const_value)) return false;
            }
            break;
            
        case NTYPE_VARIABLE:
            if (strcmp(a->var_name, b->var_name) != 0) return false;
    }
    
    return true;
}

bool tree_equals(ParsingContext *ctx, Node *a, Node *b)
{
    if (ctx == NULL || a == NULL || b == NULL) return false;
    
    if (!node_equals(ctx, a, b)) return false;
    if (a->type == NTYPE_OPERATOR)
    {
        for (size_t i = 0; i < a->num_children; i++)
        {
            if (!tree_equals(ctx, a->children[i], b->children[i])) return false;
        }
    }
    return true;
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// The following functions are used to print nodes or trees to console
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

void print_constant(ParsingContext *ctx, Node *node)
{
    if (node->type == NTYPE_CONSTANT)
    {
        char value[ctx->min_str_len];
        ctx->to_string(node->const_value, value, ctx->min_str_len);
        printf(CONST_COLOR "%s" COL_RESET, value);
    }
}

void print_variable(Node *node)
{
    if (node->type == NTYPE_VARIABLE)
    {
        printf(VAR_COLOR "%s" COL_RESET, node->var_name);
    }
}

void print_tree_visual_rec(ParsingContext *ctx, Node *node, int layer, unsigned int vert_lines, bool last_child)
{
    for (int i = 0; i < layer - 1; i++)
    {
        if (vert_lines & (1 << i))
        {
            printf("│   "); // Bit at i=1
        }
        else
        {
            printf("    "); // Bit at i=0
        }
    }
    
    if (layer != 0)
    {
        if (!last_child)
        {
            printf("├── ");
        }
        else
        {
            printf("└── ");
        }
    }
    
    switch (node->type)
    {
        case NTYPE_OPERATOR:
            printf(OP_COLOR "%s" COL_RESET "\n", node->op->name);
            for (int i = 0; i < node->num_children; i++)
            {
                bool is_last = i == node->num_children - 1;
                print_tree_visual_rec(
                    ctx,
                    node->children[i], layer + 1, is_last ? vert_lines : vert_lines | (1 << layer),
                    is_last);
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
Summary: Draws coloured tree of node to stdout
*/
void print_tree_visual(ParsingContext *ctx, Node *node)
{
    if (ctx == NULL || node == NULL) return;
    print_tree_visual_rec(ctx, node, 0, 0, true);
}

void print_tree_inline_rec(ParsingContext *ctx, Node *node, bool l, bool r)
{
    switch (node->type)
    {
        case NTYPE_CONSTANT:
            print_constant(ctx, node);
            break;
            
        case NTYPE_VARIABLE:
            print_variable(node);
            break;
            
        case NTYPE_OPERATOR:
            switch (node->op->placement)
            {
                case OP_PLACE_PREFIX:
                    if (node->op->arity == 0) l = false;
                    if (l) printf("(");
                    printf("%s", node->op->name);
                    
                    if (node->op->arity != 0)
                    {
                        if (node->children[0]->type == NTYPE_OPERATOR
                            && node->children[0]->op->precedence <= node->op->precedence)
                        {
                            printf("(");
                            print_tree_inline_rec(ctx, node->children[0], false, false);
                            printf(")");
                        }
                        else
                        {
                            print_tree_inline_rec(ctx, node->children[0], true, !l && r);
                        }
                    }
                    
                    if (l) printf(")");
                    break;
                    
                case OP_PLACE_POSTFIX:
                    if (r) printf("(");
                    
                    if (node->op->arity != 0)
                    {
                        if (node->children[0]->type == NTYPE_OPERATOR
                            && node->children[0]->op->precedence < node->op->precedence)
                        {
                            printf("(");
                            print_tree_inline_rec(ctx, node->children[0], false, false);
                            printf(")");
                        }
                        else
                        {
                            print_tree_inline_rec(ctx, node->children[0], l && !r, true);
                        }
                    }
                    
                    printf("%s", node->op->name);
                    if (r) printf(")");
                    break;
                    
                case OP_PLACE_FUNCTION:
                    printf("%s(", node->op->name);
                    for (size_t i = 0; i < node->num_children; i++)
                    {
                        print_tree_inline_rec(ctx, node->children[i], false, false);
                        if (i != node->num_children - 1) printf(", ");
                    }
                    printf(")");
                    break;
                    
                case OP_PLACE_INFIX:
                    if (node->children[0]->type == NTYPE_OPERATOR
                        && (node->children[0]->op->precedence < node->op->precedence
                            || (node->children[0]->op->precedence == node->op->precedence
                                && node->op->assoc == OP_ASSOC_RIGHT)))
                    {
                        printf("(");
                        print_tree_inline_rec(ctx, node->children[0], false, false);
                        printf(")");
                    }
                    else
                    {
                        print_tree_inline_rec(ctx, node->children[0], l, true);
                    }

                    if (strlen(node->op->name) == 1)
                    {
                        printf("%s", node->op->name);
                    }
                    else
                    {
                        printf(" %s ", node->op->name);
                    }
                    
                    if (node->children[1]->type == NTYPE_OPERATOR
                        && (node->children[1]->op->precedence < node->op->precedence
                            || (node->children[1]->op->precedence == node->op->precedence
                                && node->op->assoc == OP_ASSOC_LEFT)))
                    {
                        printf("(");
                        print_tree_inline_rec(ctx, node->children[1], false, false);
                        printf(")");
                    }
                    else
                    {
                        print_tree_inline_rec(ctx, node->children[1], true, r);
                    }
            }
            
            break;
    }
}

/*
Summary: Prints expression of node to stdout
*/
void print_tree_inline(ParsingContext *ctx, Node *node)
{
    if (ctx == NULL || node == NULL) return;
    print_tree_inline_rec(ctx, node, false, false);
}
