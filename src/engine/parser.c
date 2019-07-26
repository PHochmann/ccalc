#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "constants.h"
#include "tokenizer.h"
#include "parser.h"

#define ERROR(type) { result = type; goto exit; }

// Global vars while parsing:

static ParsingContext *ctx;
static Node **node_stack;
// NULL pointer on stack is opening parenthesis
static Operator **op_stack;
// Records number of operands for functions, or DYNAMIC_ARITY for non-functions and glue-ops
static Arity *arities;
static size_t num_nodes;
static size_t num_ops;
static ParserError result;

Operator* op_peek()
{
    return op_stack[num_ops - 1];
}

Arity arity_peek()
{
    return arities[num_ops - 1];
}

void *malloc_wrapper(size_t size)
{
    void *res = malloc(size);
    if (res == NULL) result = PERR_OUT_OF_MEMORY;
    return res;
}

bool node_push(Node *value)
{
    if (num_nodes == MAX_STACK_SIZE)
    {
        free(value);
        result = PERR_STACK_EXCEEDED;
        return false;
    }

    node_stack[num_nodes++] = value;
    return true;
}

bool node_pop(Node **out)
{
    if (num_nodes == 0)
    {
        result = PERR_MISSING_OPERAND;
        return false;
    }
    
    *out = node_stack[--num_nodes];
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
    if (op != NULL) // Construct operator-node and append children
    {
        // Functions with recorded arity of DYNAMIC_ARITY are glue-ops and shouldn't be computed as functions
        bool is_function = (arity_peek() != DYNAMIC_ARITY);
    
        // Function overloading: Find function with suitable arity
        // Actual function on op stack is only tentative with random arity (but same name)
        if (is_function)
        {
            if (op->arity != arity_peek())
            {
                char *name = op->name;
                op = ctx_lookup_function(ctx, name, arity_peek());
                
                // Fallback: Find function of dynamic arity
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

        // We try to allocate a new node and pop its children from node stack
        Node *op_node = malloc_wrapper(sizeof(Node));
        if (op_node == NULL) return false;
        *op_node = get_operator_node(op, is_function ? arity_peek() : op->arity);

        // Check if malloc of children buffer in get_operator_node failed
        if (op_node->children == NULL)
        {
            free(op_node);
            return false;
        }
        
        for (Arity i = 0; i < op_node->num_children; i++)
        {
            // Pop nodes from stack and append them in subtree
            if (!node_pop(&op_node->children[op_node->num_children - i - 1]))
            {
                // Free already appended children and new node on error
                for (Arity j = op_node->num_children - 1; j > op_node->num_children - i - 1; j--)
                {
                    free_tree(op_node->children[j]);
                }

                free(op_node->children);
                free(op_node);
                return false;
            }
        }
        
        if (!node_push(op_node))
        {
            free_tree(op_node);
            return false;
        }
    }
    
    num_ops--;
    return true;
}

// Pushing NULL means pushing an opening parenthesis
bool op_push(Operator *op)
{
    bool is_function = (op != NULL && op->placement == OP_PLACE_FUNCTION);
    
    if (op != NULL)
    {
        if (op->placement == OP_PLACE_INFIX || op->placement == OP_PLACE_POSTFIX)
        {
            OpAssociativity assoc = op->assoc;
            if (op->assoc == OP_ASSOC_BOTH) assoc = STANDARD_ASSOC;

            while (num_ops > 0
                && op_peek() != NULL
                && (op->precedence < op_peek()->precedence
                    || (op->precedence == op_peek()->precedence && assoc == OP_ASSOC_LEFT)))
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
    
    // DYNAMIC_ARITY is only Arity that will never occur
    arities[num_ops] = (is_function ? 0 : DYNAMIC_ARITY);
    op_stack[num_ops++] = op;

    // Postfix operators are never on the op_stack, because their operands are directly available
    if (op != NULL && op->placement == OP_PLACE_POSTFIX)
    {
        if (!op_pop_and_insert()) return false;
    }

    return true;
}

ParserError parse_tokens(ParsingContext *context, size_t num_tokens, char **tokens, Node **out_res)
{
    // 1. Early outs
    if (context == NULL || tokens == NULL || out_res == NULL) return PERR_ARGS_MALFORMED;

    // 2. Initialize data structures
    ctx = context;
    result = PERR_SUCCESS;
    num_ops = 0;
    num_nodes = 0;
    op_stack = malloc_wrapper(MAX_STACK_SIZE * sizeof(Operator*));
    node_stack = malloc_wrapper(MAX_STACK_SIZE * sizeof(Node*));
    arities = malloc_wrapper(MAX_STACK_SIZE * sizeof(Arity));
    if (result == PERR_OUT_OF_MEMORY) goto exit;

    // 3. Process each token
    bool await_subexpression = true;
    for (size_t i = 0; i < num_tokens; i++)
    {
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

                // If glue-op was function, we can't count operands like we normally do
                // Disable function overloading mechanism
                arities[num_ops - 1] = DYNAMIC_ARITY;
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
            await_subexpression = false;

            while (num_ops > 0 && op_peek() != NULL)
            {
                if (!op_pop_and_insert())
                {
                    ERROR(PERR_EXCESS_CLOSING_PARENTHESIS);
                }
            }
            
            if (num_ops > 0)
            {
                op_pop_and_insert();
            }
            else
            {
                ERROR(PERR_EXCESS_CLOSING_PARENTHESIS);
            }
            
            bool empty_params = (i > 0 && is_opening_parenthesis(tokens[i - 1][0]));
            if (num_ops > 0 && arity_peek() != DYNAMIC_ARITY && !empty_params)
            {
                arities[num_ops - 1]++;
                if (arity_peek() == DYNAMIC_ARITY)
                {
                    ERROR(PERR_CHILDREN_EXCEEDED);
                }
            }
            
            continue;
        }
        
        if (is_delimiter(token[0]))
        {
            while (num_ops > 0 && op_peek() != NULL)
            {
                if (!op_pop_and_insert())
                {
                    ERROR(PERR_UNEXPECTED_DELIMITER);
                }
            }
            
            // Increase operand counter
            if (num_ops > 1 && arities[num_ops - 2] != DYNAMIC_ARITY)
            {
                arities[num_ops - 2]++;
                if (arities[num_ops - 2] == DYNAMIC_ARITY)
                {
                    ERROR(PERR_CHILDREN_EXCEEDED);
                }
            }
            
            await_subexpression = true;
            continue;
        }
        // - - -
        
        // V. Is token operator?
        Operator *op = NULL;
        
        // Infix, Prefix (await=true) -> Function (false), Leaf (false), Prefix (true)
        // Function, Leaf, Postfix (await=false) -> Infix (true), Postfix (false)
        if (await_subexpression)
        {
            op = ctx_lookup_op(ctx, token, OP_PLACE_FUNCTION);
            if (op != NULL) // Function operator found
            {
                if (!op_push(op)) goto exit;
                // Handle unary functions without parenthesis (e.g. "sin2")
                // If function is last token its arity will be set to 0
                if (i < num_tokens - 1 && !is_opening_parenthesis(tokens[i + 1][0]))
                {
                    arities[num_ops - 1] = 1;
                }
                await_subexpression = true;
                continue;
            }
            
            op = ctx_lookup_op(ctx, token, OP_PLACE_PREFIX);
            if (op != NULL) // Prefix operator found
            {
                if (!op_push(op)) goto exit;
                // Constants are modeled as prefix operators with arity of 0 and don't await a subexpression
                await_subexpression = (op->arity != 0);
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
            ERROR(PERR_UNEXPECTED_SUBEXPRESSION);
        }
        
        // VI. Token must be variable or constant (leaf)
        Node *node = malloc_wrapper(sizeof(Node));
        void *constant = malloc_wrapper(ctx->value_size);

        if (result == PERR_OUT_OF_MEMORY)
        {
            free(node);
            free(constant);
            goto exit;
        }

        if (ctx->try_parse(token, constant)) // Is token constant?
        {
            *node = get_constant_node(constant);
        }
        else // Token must be variable
        {
            free(constant);
            char *name = malloc_wrapper((tok_len + 1) * sizeof(char));
            if (result == PERR_OUT_OF_MEMORY) goto exit;
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
            ERROR(PERR_EXCESS_OPENING_PARENTHESIS);
        }
        else
        {
            if (!op_pop_and_insert()) goto exit;
        }
    }
    
    // 6. Build result and return value
    switch (num_nodes)
    {
        case 0:
            result = PERR_EMPTY; // We haven't constructed a single node
            break;

        case 1:
            *out_res = node_stack[0]; // We successfully constructed a single AST
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
Returns: Result code to indicate whether string was parsed successfully or which error occurred
*/
ParserError parse_input(ParsingContext *context, char *input, bool pad_parentheses, Node **out_res)
{
    size_t num_tokens;
    char *tokens[MAX_TOKENS];

    // Parsing
    if (!tokenize(context, input, pad_parentheses, &num_tokens, tokens)) return PERR_MAX_TOKENS_EXCEEDED;
    ParserError result = parse_tokens(context, num_tokens, tokens, out_res);

    // Cleanup
    for (size_t i = 0; i < num_tokens; i++) free(tokens[i]);
    return result;
}
