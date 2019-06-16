#include <string.h>
#include <stdio.h>
#include <sys/types.h>

#include "node.h"

Node get_node(NodeType type)
{
    Node res;
    res.type = type;
    return res;
}

// Returns a new node of type NTYPE_VARIABLE and prepares its attributes
Node get_variable_node(char *var_name)
{
    Node res = get_node(NTYPE_VARIABLE);
    res.var_name = var_name;
    return res;
}

// Returns a new node of type NTYPE_CONSTANT and prepares its attributes
Node get_constant_node(void *value)
{
    Node res = get_node(NTYPE_CONSTANT);
    res.const_value = value;
    return res;
}

// Returns a new node of type NTYPE_OPERATOR and prepares its attributes
Node get_operator_node(Operator *op, Arity num_children)
{
    Node res = get_node(NTYPE_OPERATOR);
    res.op = op;
    res.num_children = num_children;
    res.children = malloc(num_children * sizeof(Node*));
    return res;
}

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

/* Returns true iff variable node exists in tree
   False indicates safe evaluation */
bool tree_contains_vars(Node *tree)
{
    if (tree == NULL) return false;
    
    switch (tree->type)
    {
        case NTYPE_CONSTANT:
            return false;
            
        case NTYPE_VARIABLE:
            return true;
            
        case NTYPE_OPERATOR:
            for (Arity i = 0; i < tree->num_children; i++)
            {
                if (tree_contains_vars(tree->children[i])) return true;
            }
            return false;
    }
    
    return false;
}

/*
Summary: Lists all variable nodes of given names (e.g. to replace them)
Params
    out_instances: Buffer to nodes. Must hold max_var
*/
int tree_get_var_instances(Node *tree, char *variable, Node **out_instances)
{
    if (tree == NULL) return -1;
    
    Node *node_stack[MAX_STACK_SIZE];
    node_stack[0] = tree;
    int res_count = 0;
    size_t stack_count = 1;
    
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
                for (ssize_t i = curr_node->num_children - 1; i >= 0; i--)
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
Returns: Length of out_variables (i.e. count of variables in tree without duplicates)
Params
    out_variables must hold at least MAX_VAR_COUNT char-pointers or NULL to only count variables
        (strings are copied, don't forget to call free on each of out_variables' items)
*/
int tree_list_vars(Node *tree, char **out_variables)
{
    if (tree == NULL) return -1;
    
    int res_count = 0;
    
    Node *node_stack[MAX_STACK_SIZE];
    node_stack[0] = tree;
    size_t stack_count = 1;

    char *variables[MAX_VAR_COUNT];

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
                    if (strcmp(variables[i], curr_node->var_name) == 0)
                    {
                        flag = true;
                        break;
                    }
                }
                if (flag) break;
                
                // Buffer overflow protection
                if (res_count == MAX_VAR_COUNT) return -1;
                
                variables[res_count] = malloc(strlen(curr_node->var_name) + 1);
                strcpy(variables[res_count++], curr_node->var_name);
                break;
                
            case NTYPE_OPERATOR:
                for (ssize_t i = curr_node->num_children - 1; i >= 0; i--)
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
    
    if (out_variables != NULL)
    {
        for (int i = 0; i < res_count; i++)
        {
            out_variables[i] = variables[i];
        }
    }
    else
    {
        // When out_variables is NULL, we only want to count variables
        for (int i = 0; i < res_count; i++)
        {
            free(variables[i]);
        }
    }

    return res_count;
}

/*
Summary: Substitutes any occurrence of a variable with certain name with a given subtree
Returns: Number of occurrences of variable
*/
int tree_substitute_var(ParsingContext *ctx, Node *tree, Node *tree_to_copy, char *var_name)
{
    if (ctx == NULL || tree == NULL || tree_to_copy == NULL || var_name == NULL) return 0;
    
    Node *var_instances[MAX_VAR_COUNT];
    int inst_count = tree_get_var_instances(tree, var_name, var_instances);
    
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
    Node res;
    
    switch (tree->type)
    {
        case NTYPE_OPERATOR:
            res = get_operator_node(tree->op, tree->num_children);
            for (Arity i = 0; i < tree->num_children; i++)
            {
                res.children[i] = malloc(sizeof(Node));
                *(res.children[i]) = tree_copy(ctx, tree->children[i]);
            }
            break;
        
        case NTYPE_CONSTANT:
            res = get_constant_node(malloc(ctx->value_size));
            for (size_t i = 0; i < ctx->value_size; i++)
            {
                *((char*)(res.const_value) + i) = *((char*)(tree->const_value) + i);
            }
            break;
            
        case NTYPE_VARIABLE:
            res = get_variable_node(malloc((strlen(tree->var_name) + 1) * sizeof(char)));
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
Summary: Checks if two trees represent the same expression, i.e. all nodes in "a" are equal to their respective node in "b"
*/
bool tree_equals(ParsingContext *ctx, Node *a, Node *b)
{
    if (ctx == NULL || a == NULL || b == NULL) return false;
    
    if (!node_equals(ctx, a, b)) return false;

    if (a->type == NTYPE_OPERATOR)
    {
        for (Arity i = 0; i < a->num_children; i++)
        {
            if (!tree_equals(ctx, a->children[i], b->children[i])) return false;
        }
    }
    return true;
}
