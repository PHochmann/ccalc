#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "constants.h"
#include "memory.h"
#include "tokenizer.h"
#include "parser.h"

// Global vars while parsing
static ParsingContext *ctx;
static Node **node_stack;
static Operator **op_stack;
static int *arities;
static int num_nodes;
static int num_ops;
static ParserError result;

/* Check if stack is empty before calling! */
Operator* op_peek()
{
    return op_stack[num_ops - 1];
}

bool node_push(Node *value)
{
    if (num_nodes == MAX_STACK_SIZE)
    {
        free(value);
        result = PERR_STACK_EXCEEDED;
        return false;
    }

    node_stack[num_nodes] = value;
    num_nodes++;
    
    return true;
}

bool node_pop(Node **out)
{
    if (num_nodes == 0)
    {
        result = PERR_MISSING_OPERAND;
        return false;
    }
    
    num_nodes--;
    *out = node_stack[num_nodes];
    
    return true;
}

bool op_pop_and_insert()
{
    if (num_ops == 0)
    {
        result = PERR_MISSING_OPERATOR;
        return false;
    }
    
    Operator *op = op_peek();
    bool is_function = (op != NULL && op->placement == OP_PLACE_FUNCTION);
    
    // Function overloading: Find function with suitable arity
    if (is_function)
    {
        if (op->arity != arities[num_ops - 1])
        {
            char *name = op->name;
            op = ctx_lookup_function(ctx, name, arities[num_ops - 1]);
            
            // Fallback: Find function of dynamic aritiy
            if (op == NULL)
            {
                op = ctx_lookup_function(ctx, name, DYNAMIC_ARITY);
            }
            
            if (op == NULL)
            {
                result = PERR_FUNCTION_WRONG_ARITY;
                return false;
            }
        }
    }
    
    if (op != NULL) // Construct operator-node and append children
    {
        Node *op_node = malloc(sizeof(Node));
        *op_node = get_operator_node(op);
        op_node->num_children = is_function ? arities[num_ops - 1] : op->arity;
        
        if (op_node->num_children > MAX_CHILDREN)
        {
            result = PERR_EXCEEDED_MAX_CHILDREN;
            free(op_node);
            return false;
        }
        
        for (int i = 0; i < op_node->num_children; i++)
        {
            if (!node_pop(&(op_node->children[op_node->num_children - i - 1])))
            {
                // Free already appended children and new node
                for (int j = op_node->num_children - 1; j > op_node->num_children - i - 1; j--)
                {
                    free_tree(op_node->children[j]);
                }
                free(op_node);
                return false;
            }
        }
        
        if (!node_push(op_node)) return false;
    }
    
    num_ops--;
    return true;
}

bool op_push(Operator *op)
{
    bool is_function = (op != NULL && op->placement == OP_PLACE_FUNCTION);
    
    if (op != NULL)
    {
        if (op->placement == OP_PLACE_INFIX || op->placement == OP_PLACE_POSTFIX)
        {
            while (num_ops > 0
                && op_peek() != NULL
                && (op->precedence < op_peek()->precedence
                    || (op->precedence == op_peek()->precedence
                        && (op->assoc == OP_ASSOC_LEFT || op->assoc == OP_ASSOC_BOTH))))
            {
                if (!op_pop_and_insert()) return false;
            }
        }
    }
    
    if (num_ops == MAX_STACK_SIZE)
    {
        result = PERR_STACK_EXCEEDED;
        return false;
    }
    
    arities[num_ops] = (is_function ? 0 : -1);
    op_stack[num_ops] = op;
    num_ops++;

    // Postfix operators are never on the op_stack, because their operands are directly available
    if (op != NULL && op->placement == OP_PLACE_POSTFIX)
    {
        if (!op_pop_and_insert()) return false;
    }

    return true;
}

ParserError parse_tokens(ParsingContext *context, char **tokens, int num_tokens, Node **res)
{
    // 1. Early outs
    if (context == NULL || tokens == NULL || res == NULL) return PERR_ARGS_MALFORMED;

    // 2. Initialize data structures
    ctx = context;
    result = PERR_SUCCESS;
    num_ops = 0;
    num_nodes = 0;
    op_stack = malloc(MAX_STACK_SIZE * sizeof(Operator*));
    node_stack = malloc(MAX_STACK_SIZE * sizeof(Node*));
    arities = malloc(MAX_STACK_SIZE * sizeof(int));

    // 3. Process each token
    bool await_subexpression = true;
    for (int i = 0; i < num_tokens; i++)
    {
        if (result != PERR_SUCCESS) goto exit;
        
        char *token = tokens[i];
        size_t tok_len = strlen(token);
        
        // II. Does glue-op need to be inserted?
        if (!await_subexpression && ctx->glue_op != NULL)
        {
            if (!is_closing_parenthesis(token[0])
                && !is_delimiter(token[0])
                && ctx_lookup_op(ctx, token, OP_PLACE_INFIX) == NULL
                && ctx_lookup_op(ctx, token, OP_PLACE_POSTFIX) == NULL)
            {
                if (!op_push(ctx->glue_op)) goto exit;
                await_subexpression = true;
            }
        }
        
        // III. Is token opening parenthesis?
        if (is_opening_parenthesis(token[0]))
        {
            if (!op_push(NULL)) goto exit;
            await_subexpression = true;
            continue;
        }

        // IV. Is token closing parenthesis or argument delimiter?
        if (is_closing_parenthesis(token[0]))
        {
            while (num_ops > 0 && op_peek() != NULL)
            {
                if (!op_pop_and_insert())
                {
                    result = PERR_UNEXPECTED_CLOSING_PARENTHESIS;
                    goto exit;
                }
            }
            
            if (num_ops > 0)
            {
                op_pop_and_insert();
            }
            else
            {
                result = PERR_UNEXPECTED_CLOSING_PARENTHESIS;
                goto exit;
            }
            
            bool empty_params = (i > 0 && is_opening_parenthesis(tokens[i - 1][0]));
            if (num_ops > 0 && arities[num_ops - 1] != -1 && !empty_params)
            {
                arities[num_ops - 1]++;
            }
            
            await_subexpression = false;
            
            continue;
        }
        
        if (is_delimiter(token[0]))
        {
            while (num_ops > 0 && op_peek() != NULL)
            {
                if (!op_pop_and_insert())
                {
                    result = PERR_UNEXPECTED_DELIMITER;
                    goto exit;
                }
            }
            
            // Increase operand counter
            if (num_ops > 1 && arities[num_ops - 2] != -1)
            {
                arities[num_ops - 2]++;
            }
            
            await_subexpression = true;
            
            continue;
        }
        // - - -
        
        // V. Is token operator?
        Operator *op = NULL;
        if (await_subexpression)
        {
            op = ctx_lookup_op(ctx, token, OP_PLACE_FUNCTION);
            if (op != NULL) // Function operator found
            {
                if (!op_push(op)) goto exit;
                await_subexpression = true;
                
                // Handle unary functions without parenthesis (e.g. "sin2")
                // If function is last token its arity will be set to 0
                if (i != num_tokens - 1)
                {
                    if (!is_opening_parenthesis(tokens[i + 1][0]))
                    {
                        arities[num_ops - 1] = 1;
                    }
                }
                
                continue;
            }
            
            op = ctx_lookup_op(ctx, token, OP_PLACE_PREFIX);
            if (op != NULL) // Prefix operator found
            {
                if (!op_push(op)) goto exit;
                await_subexpression = (op->arity != 0); // Constants don't await subexpression
                continue;
            }
        }
        else
        {
            op = ctx_lookup_op(ctx, token, OP_PLACE_INFIX);
            if (op != NULL) // Infix operator found
            {
                if (!op_push(op)) goto exit;
                await_subexpression = true;
                continue;
            }
            
            op = ctx_lookup_op(ctx, token, OP_PLACE_POSTFIX);
            if (op != NULL) // Postfix operator found
            {
                if (!op_push(op)) goto exit;
                await_subexpression = false;
                continue;
            }
            
            // We can fail here: no more tokens processable (no glue-op)
            result = PERR_UNEXPECTED_SUBEXPRESSION;
            goto exit;
        }
        
        // VI. Token must be variable or constant (leaf)
        Node *node = malloc(sizeof(Node));
        void *constant = malloc(ctx->value_size);
        if (ctx->try_parse(token, constant)) // Is token constant?
        {
            *node = get_constant_node(constant);
        }
        else // Token must be variable
        {
            free(constant);
            char *name = malloc((tok_len + 1) * sizeof(char));
            strcpy(name, token);
            *node = get_variable_node(name);
        }
        await_subexpression = false;
        
        if (!node_push(node)) goto exit;
    }
    
    // 5. Pop all remaining operators
    while (num_ops > 0)
    {
        if (op_peek() == NULL)
        {
            result = PERR_UNEXPECTED_OPENING_PARENTHESIS;
            goto exit;
        }
        if (!op_pop_and_insert()) goto exit;
    }
    
    // 6. Build result and return value
    switch (num_nodes)
    {
        case 0:
            result = PERR_EMPTY; // We haven't constructed a single node
            break;
        case 1:
            result = PERR_SUCCESS; // We successfully constructed a single AST
            *res = node_stack[0];
            break;
        default:
            result = PERR_MISSING_OPERATOR; // We have multiple ASTs (need glue-op)
    }
    
    exit:
    
    // If parsing wasn't successful, free partial results
    if (result != PERR_SUCCESS)
    {
        while (num_nodes > 0)
        {
            free_tree(node_stack[num_nodes - 1]);
            num_nodes--;
        }
    }

    free(node_stack);
    free(op_stack);
    free(arities);

    return result;
}

/*
Summary: Parses string to abstract syntax tree with operators of given context
Returns: result code to indicate whether string was parsed successfully or which result occured
*/
ParserError parse_input(ParsingContext *context, char *input, Node **res)
{
    int num_tokens;
    char **tokens;
    if (!tokenize(context, input, &tokens, &num_tokens)) return PERR_MAX_TOKENS_EXCEEDED;
    ParserError result = parse_tokens(context, tokens, num_tokens, res);
    for (int i = 0; i < num_tokens; i++) free(tokens[i]);
    free(tokens);
    return result;
}
