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
    if (ctx == NULL || tree == NULL || tree_to_copy == NULL || var_name == NULL) return 0;
    
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
    free_tree_preserved(destination);
    *destination = new_node;
}

/*
Summary: Copies tree, tree_equals will be true of copy. Source tree can be safely free'd afterwards.
*/
Node tree_copy(ParsingContext *ctx, Node *tree)
{
    //if (ctx == NULL || tree == NULL) ...?

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
    if (a == NULL || b == NULL) return false;

    for (size_t i = 0; i < value_size; i++)
    {
        if (((char*)a)[i] != ((char*)b)[i]) return false;
    }
    
    return true;
}

/*
Summary: Checks if two nodes are equal, i.e. have the same type and respective further values
*/
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

/*
Summary: Checks if two trees represent the same expression, i.e. all nodes in 'a' are equal to their respective node in 'b'
*/
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
