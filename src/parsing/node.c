#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include "node.h"

// When traversing a tree, a stack must be maintained
const size_t MAX_TREE_SEARCH_STACK_SIZE = 100;

// Maximal amount of variables nodes allowed in tree
const size_t MAX_VAR_COUNT = 20;

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
Node get_constant_node(double value)
{
    Node res = get_node(NTYPE_CONSTANT);
    res.const_value = value;
    return res;
}

/* Returns a new node of type NTYPE_OPERATOR and prepares its attributes
   Also mallocs buffer for num_children Nodes */
Node get_operator_node(Operator *op, size_t num_children)
{
    Node res = get_node(NTYPE_OPERATOR);
    res.op = op;
    res.num_children = num_children;
    res.children = malloc(num_children * sizeof(Node*));
    return res;
}

Node *malloc_node(Node node)
{
    Node *res = malloc(sizeof(Node));
    *res = node;
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
            break;
        case NTYPE_VARIABLE:
            free(tree->var_name);
    }
}

Node *tree_search(ParsingContext *ctx, Node *tree, bool (*predicate)(ParsingContext*, Node*))
{
    // Basic case
    if (predicate(ctx, tree)) return tree;
    
    // Recursive case
    if (tree->type == NTYPE_OPERATOR)
    {
        for (size_t i = 0; i < tree->num_children; i++)
        {
            Node *recursive_res = tree_search(ctx, tree->children[i], predicate);
            if (recursive_res != NULL) return recursive_res;
        }
    }
    
    return NULL;
}

/* Returns number of variable nodes in tree
   False indicates safe evaluation */
size_t tree_count_vars(Node *tree)
{
    if (tree == NULL) return false;

    switch (tree->type)
    {
        case NTYPE_CONSTANT:
            return 0;

        case NTYPE_VARIABLE:
            return 1;

        case NTYPE_OPERATOR:
        {
            size_t sum = 0;
            for (size_t i = 0; i < tree->num_children; i++)
            {
                sum += tree_count_vars(tree->children[i]);
            }
            return sum;
        }

        default:
            return 0;
    }
}

bool tree_count_vars_distinct(Node *tree, size_t *out_num_variables)
{
    // out-discard pattern

    char *vars[MAX_VAR_COUNT];
    if (tree_list_vars(tree, out_num_variables, vars))
    {
        for (size_t i = 0; i < *out_num_variables; i++)
        {
            free(vars[i]);
        }
        return true;
    }
    return false;
}

/*
Summary: Lists all variable nodes of given names (e.g. to replace them)
Returns: False if NULL in args, or MAX_TREE_SEARCH_STACK_SIZE or MAX_VAR_COUNT exceeded. True otherwise.
Params
    out_instances: Buffer to nodes. Must hold at least MAX_VAR_COUNT Node-pointers.
*/
bool tree_get_var_instances(Node *tree, char *var_name, size_t *out_num_instances, Node **out_instances)
{
    if (tree == NULL || var_name == NULL || out_num_instances == NULL || out_instances == NULL) return false;
    
    Node *node_stack[MAX_TREE_SEARCH_STACK_SIZE];
    size_t num_nodes = 1;
    node_stack[0] = tree;
    *out_num_instances = 0;
    
    while (num_nodes > 0)
    {
        Node *curr_node = node_stack[--num_nodes];
        switch (curr_node->type)
        {
            case NTYPE_VARIABLE:
                if (strcmp(curr_node->var_name, var_name) == 0)
                {
                    if (*out_num_instances == MAX_VAR_COUNT) return false;
                    out_instances[(*out_num_instances)++] = curr_node;
                }
                break;
                
            case NTYPE_OPERATOR:
                for (size_t i = 0; i < curr_node->num_children; i++)
                {
                    if (num_nodes == MAX_TREE_SEARCH_STACK_SIZE) return false;
                    node_stack[num_nodes++] = curr_node->children[curr_node->num_children - 1 - i];
                }
                break;
                
            case NTYPE_CONSTANT:
                break;
        }
    }
    
    return true;
}

bool tree_count_var_instances(Node *tree, char *var_name, size_t *out_num_instances)
{
    // out-discard pattern

    Node *instances[MAX_VAR_COUNT];
    return tree_get_var_instances(tree, var_name, out_num_instances, instances);
}

/*
Summary: Lists all variable names in tree.
Returns: False if stack exceeded or MAX_VAR_COUNT exceeded
Params
    out_num_variables: Length of out_variables (i.e. count of variables in tree without duplicates)
    out_variables: Must hold at least MAX_VAR_COUNT char-pointers or NULL to only count variables
        (strings are copied, don't forget to call free on each of out_variables' items)
*/
bool tree_list_vars(Node *tree, size_t *out_num_variables, char **out_variables)
{
    if (tree == NULL || out_num_variables == NULL || out_variables == NULL) return false;
    
    Node *node_stack[MAX_TREE_SEARCH_STACK_SIZE];
    size_t num_nodes = 1;
    node_stack[0] = tree;
    *out_num_variables = 0;

    while (num_nodes != 0)
    {
        Node *curr_node = node_stack[--num_nodes];
        bool flag = false; // To break twice
        
        switch (curr_node->type)
        {
            case NTYPE_VARIABLE:
                for (size_t i = 0; i < *out_num_variables; i++)
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
                if (*out_num_variables == MAX_VAR_COUNT) goto error;
                
                out_variables[*out_num_variables] = malloc(strlen(curr_node->var_name) + 1);
                strcpy(out_variables[(*out_num_variables)++], curr_node->var_name);
                break;
                
            case NTYPE_OPERATOR:
                for (size_t i = 0; i < curr_node->num_children; i++)
                {
                    if (num_nodes == MAX_TREE_SEARCH_STACK_SIZE) goto error;
                    node_stack[num_nodes++] = curr_node->children[curr_node->num_children - 1 - i];
                }
                break;

            case NTYPE_CONSTANT:
                break;
        }
    }

    return true;
    
    error:
    // Free partial results
    for (size_t i = 0; i < *out_num_variables; i++)
    {
        free(out_variables[i]);
    }
    return false;
}

/*
Summary: Substitutes any occurrence of a variable with certain name with a given subtree
Returns: True if succeeded, otherwise false (due to exceeded MAX_STACK_SIZE etc.)
*/
bool tree_substitute_var(ParsingContext *ctx, Node *tree, Node *tree_to_copy, char *var_name)
{
    if (ctx == NULL || tree == NULL || tree_to_copy == NULL || var_name == NULL) return false;
    
    Node *vars[MAX_VAR_COUNT];
    size_t num_vars;

    if (!tree_get_var_instances(tree, var_name, &num_vars, vars)) return false;

    for (size_t i = 0; i < num_vars; i++)
    {
        tree_replace(vars[i], tree_copy(ctx, tree_to_copy));
    }
    
    return true;
}

/*
Summary: frees all child-trees and replaces root value-wise
Params
    new_node: Tree, destination is replaced with. You may want to copy it before.
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
            for (size_t i = 0; i < tree->num_children; i++)
            {
                res.children[i] = malloc(sizeof(Node));
                *(res.children[i]) = tree_copy(ctx, tree->children[i]);
            }
            break;
        
        case NTYPE_CONSTANT:
            res = get_constant_node(tree->const_value);
            break;
            
        case NTYPE_VARIABLE:
            res = get_variable_node(malloc((strlen(tree->var_name) + 1) * sizeof(char)));
            strcpy(res.var_name, tree->var_name);
            break;
    }
    
    return res;
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
            return a->op == b->op && a->num_children == b->num_children;
        case NTYPE_CONSTANT:
            return a->const_value == b->const_value;
        case NTYPE_VARIABLE:
            return strcmp(a->var_name, b->var_name) == 0;
    }
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
        for (size_t i = 0; i < a->num_children; i++)
        {
            if (!tree_equals(ctx, a->children[i], b->children[i])) return false;
        }
    }

    return true;
}
