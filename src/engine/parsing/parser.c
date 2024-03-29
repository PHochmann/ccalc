#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "tokenizer.h"
#include "parser.h"
#include "../../util/string_util.h"
#include "../../util/console_util.h"

#define VECTOR_STARTSIZE 10

// Do not use this macro in auxiliary functions!
#define ERROR(error_type) {\
    state.result.type = error_type;\
    goto exit;\
}

// Represents an operator (with metadata) while being parsed
struct OpData
{
    const Operator *op; // Pointer to operator in context, NULL denotes opening parenthesis
    size_t arity;       // Records number of operands to pop
    size_t token;
};

// Encapsulates current state of shunting-yard algo. to be communicated to auxiliary functions
struct ParserState
{
    const ParsingContext *ctx; // Contains operators and glue-op
    Vector vec_nodes;          // Constructed nodes
    Vector vec_ops;            // Parsed operators
    ParserError result;        // Success when no error occurred
    size_t curr_tok;           // Current index of token
};

// Attempts to parse a substring to a double
bool try_parse_constant(const char *in, double *out)
{
    char *end;
    *out = strtod(in, &end);
    return *end == '\0';
}

// Returns op_data on top of stack
struct OpData *op_peek(struct ParserState *state)
{
    return vec_peek(&state->vec_ops);
}

void node_push(struct ParserState *state, Node *node)
{
    VEC_PUSH_ELEM(&state->vec_nodes, Node*, node);
}

bool node_pop(struct ParserState *state, Node **out)
{
    Node **popped = vec_pop(&state->vec_nodes);
    if (popped == NULL) return false;
    *out = *popped;
    return true;
}

bool op_pop_and_insert(struct ParserState *state)
{
    struct OpData *op_data = (struct OpData*)vec_pop(&state->vec_ops);
    const Operator *op = op_data->op;

    if (op != NULL) // Construct operator-node and append children
    {
        if (op->arity != OP_DYNAMIC_ARITY && op->arity != op_data->arity)
        {
            state->result.type = PERR_FUNCTION_WRONG_ARITY;
            state->result.additional_data[0] = op_data->arity;
            state->result.additional_data[1] = op->arity;
            state->result.error_token = op_data->token;
            return false;
        }

        // We try to allocate a new node and pop its children from node stack
        Node *op_node = malloc_operator_node(op, op_data->arity, op_data->token);
        
        for (size_t i = 0; i < get_num_children(op_node); i++)
        {
            // Pop nodes from stack and append them in subtree
            if (!node_pop(state, get_child_addr(op_node, get_num_children(op_node) - i - 1)))
            {
                state->result.error_token = op_data->token;
                // Free already appended children and new node on error
                // Caller must set error in state
                free_tree(op_node);
                return false;
            }
        }
        
        node_push(state, op_node);
    }

    return true;
}

bool op_push(struct ParserState *state, struct OpData op_d)
{
    const Operator *op = op_d.op;
    
    if (op != NULL)
    {
        // Shunting-yard algorithm: Pop until operator of higher precedence or '(' is on top of stack 
        if (op->placement == OP_PLACE_INFIX || op->placement == OP_PLACE_POSTFIX)
        {
            while (op_peek(state) != NULL
                && op_peek(state)->op != NULL
                && (op->precedence < op_peek(state)->op->precedence
                    || (op->precedence == op_peek(state)->op->precedence && op->assoc == OP_ASSOC_LEFT)))
            {
                if (!op_pop_and_insert(state)) return false;
            }
        }
    }

    VEC_PUSH_ELEM(&state->vec_ops, struct OpData, op_d);
    return true;
}

// Pushes actual operator on op_stack. op must not be NULL!
bool push_operator(struct ParserState *state, const Operator *op)
{
    // Set arity of functions to 0 and enable operand counting
    struct OpData opData;

    if (op->placement == OP_PLACE_FUNCTION)
    {
        opData = (struct OpData){ op, 0, state->curr_tok };
    }
    else
    {
        opData = (struct OpData){ op, op->arity, state->curr_tok };
    }

    return op_push(state, opData);
}

// Pushes opening parenthesis on op_stack
bool push_opening_parenthesis(struct ParserState *state)
{
    return op_push(state, (struct OpData){ NULL, OP_DYNAMIC_ARITY, state->curr_tok });
}

// out_res can be NULL if you only want to check if an error occurred
ParserError parse_tokens(const ParsingContext *ctx, size_t num_tokens, const char **tokens, Node **out_res)
{
    // 1. Early outs
    if (ctx == NULL || tokens == NULL) return (ParserError){ .type = PERR_ARGS_MALFORMED };

    // 2. Initialize state
    struct ParserState state = {
        .ctx       = ctx,
        .result    = (ParserError){ .type = PERR_SUCCESS, .error_token = SIZE_MAX },
        .vec_nodes = vec_create(sizeof(Node*), VECTOR_STARTSIZE),
        .vec_ops   = vec_create(sizeof(struct OpData), VECTOR_STARTSIZE)
    };

    // 3. Process each token
    bool await_infix = false;  // Or postfix, or delimiter, or closing parenthesis
    bool await_params = false; // When a function parameter list needs to follow
    for (size_t i = 0; i < num_tokens; i++)
    {
        const char *token = tokens[i];
        state.curr_tok = i;

        // First: Ignore any whitespace-tokens
        if (is_space(token[0]))
        {
            continue;
        }
        
        // I. Does glue-op need to be inserted?
        if (await_infix && state.ctx->glue_op != NULL)
        {
            if (!is_closing_parenthesis(token)
                && !is_delimiter(token)
                && ctx_lookup_op(state.ctx, token, OP_PLACE_INFIX) == NULL
                && ctx_lookup_op(state.ctx, token, OP_PLACE_POSTFIX) == NULL)
            {
                if (!push_operator(&state, state.ctx->glue_op)) goto exit;
                // Arity of 2 needed for DYNAMIC_ARITY functions set as glue-op
                op_peek(&state)->arity = 2;
                await_infix = false;
            }
        }
        
        // II. Is token opening parenthesis?
        if (is_opening_parenthesis(token))
        {
            if (!await_infix)
            {
                await_params = false;
                if (!push_opening_parenthesis(&state)) goto exit;
                continue;
            }
            else
            {
                ERROR(PERR_EXPECTED_INFIX);
            }
        }
        else
        {
            if (await_params)
            {
                ERROR(PERR_EXPECTED_PARAM_LIST);
            }
        }

        // III. Is token closing parenthesis or argument delimiter?
        if (is_closing_parenthesis(token))
        {
            if (!await_infix &&
                (op_peek(&state) == NULL || op_peek(&state)->op != NULL))
            {
                ERROR(PERR_UNEXPECTED_CLOSING_PAREN);
            }

            // Pop ops until opening parenthesis on op-stack
            while (op_peek(&state) != NULL && op_peek(&state)->op != NULL)
            {
                if (!op_pop_and_insert(&state))
                {
                    goto exit;
                }
            }
            
            if (op_peek(&state) != NULL)
            {
                // Remove opening parenthesis on top of op-stack
                op_pop_and_insert(&state);
            }
            else
            {
                // We did not stop because an opening parenthesis was found, but because op-stack was empty
                state.result.error_token = i;
                ERROR(PERR_UNEXPECTED_CLOSING_PAREN);
            }

            // Increment operand count one last time if it was not the empty parameter list.
            if (await_infix)
            {
                if (op_peek(&state) != NULL
                    && op_peek(&state)->op != NULL
                    && op_peek(&state)->op->placement == OP_PLACE_FUNCTION)
                {
                    op_peek(&state)->arity++;
                }
            }
            else
            {
                if (op_peek(&state) == NULL // '1,'
                    || op_peek(&state)->op == NULL // 'f(())'
                    || op_peek(&state)->op->placement != OP_PLACE_FUNCTION // '()' but not empty parameter list
                    || op_peek(&state)->arity != 0) // 'f(x,)'
                {
                    ERROR(PERR_UNEXPECTED_CLOSING_PAREN);
                }
            }
            
            
            await_infix = true;
            continue;
        }
        
        if (is_delimiter(token))
        {
            if (!await_infix)
            {
                ERROR(PERR_UNEXPECTED_DELIMITER);
            }

            // Pop ops until opening parenthesis on op-stack
            while (op_peek(&state) != NULL && op_peek(&state)->op != NULL)
            {
                if (!op_pop_and_insert(&state))
                {
                    goto exit;
                }
            }

            // Increment arity counter for function whose parameter list this delimiter is in
            if (vec_count(&state.vec_ops) > 1)
            {
                struct OpData *op_data = ((struct OpData*)vec_get(&state.vec_ops, vec_count(&state.vec_ops) - 2));
                if (op_data->op->placement == OP_PLACE_FUNCTION)
                {
                    op_data->arity++;
                }
                else
                {
                    ERROR(PERR_UNEXPECTED_DELIMITER);
                }
            }
            else
            {
                ERROR(PERR_UNEXPECTED_DELIMITER);
            }
              
            await_infix = false;
            continue;
        }
        // - - -
        
        // IV. Is token operator?
        const Operator *op = NULL;
        
        // Infix, Prefix, Delimiter (await=false) -> Function (true), Leaf (true), Prefix (false)
        // Function, Leaf, Postfix (await=true) -> Infix (false), Postfix (true), Delimiter (false)
        if (!await_infix)
        {
            op = ctx_lookup_op(state.ctx, token, OP_PLACE_FUNCTION);
            if (op != NULL) // Function operator found
            {
                if (!push_operator(&state, op)) goto exit;

                // Directly pop constant functions
                if (op->arity == 0)
                {
                    if (!op_pop_and_insert(&state)) goto exit;
                    await_infix = true;
                }
                else
                {
                    await_infix = false;
                    await_params = true;
                }
                continue;
            }
            
            op = ctx_lookup_op(state.ctx, token, OP_PLACE_PREFIX);
            if (op != NULL) // Prefix operator found
            {
                if (!push_operator(&state, op)) goto exit;
                await_infix = false;
                continue;
            }
        }
        else
        {
            op = ctx_lookup_op(state.ctx, token, OP_PLACE_INFIX);
            if (op != NULL) // Infix operator found
            {
                if (!push_operator(&state, op)) goto exit;
                await_infix = false;
                continue;
            }
            
            op = ctx_lookup_op(state.ctx, token, OP_PLACE_POSTFIX);
            if (op != NULL) // Postfix operator found
            {
                if (!push_operator(&state, op)) goto exit;
                // Postfix operators are never on the op_stack because their operands are directly available
                op_pop_and_insert(&state);
                await_infix = true;
                continue;
            }
            
            // We can fail here: no more tokens processable (no glue-op)
            ERROR(PERR_EXPECTED_INFIX);
        }
        
        // V. Token must be variable or constant (leaf)
        Node *node;

        // Is token constant?
        double const_val;
        if (try_parse_constant(token, &const_val))
        {
            node = malloc_constant_node(const_val, i);
        }
        else // Token must be variable
        {
            // Check if string has the same name as an infix or a postfix operator and fail
            // to not to confuse the user
            if (ctx_lookup_op(state.ctx, token, OP_PLACE_INFIX) != NULL
                || ctx_lookup_op(state.ctx, token, OP_PLACE_POSTFIX) != NULL)
            {
                ERROR(PERR_UNEXPECTED_INFIX);
            }

            if (!is_letter(token[0]))
            {
                ERROR(PERR_UNEXPECTED_CHARACTER);
            }

            node = malloc_variable_node(token, 0, i);
        }

        await_infix = true;
        node_push(&state, node);
    }

    state.curr_tok = num_tokens;

    if (await_params)
    {
        ERROR(PERR_EXPECTED_PARAM_LIST);
    }
    if (!await_infix)
    {
        ERROR(PERR_UNEXPECTED_END_OF_EXPR);
    }
    
    // 4. Pop all remaining operators
    while (op_peek(&state) != NULL)
    {
        if (op_peek(&state)->op == NULL)
        {
            ERROR(PERR_EXCESS_OPENING_PAREN);
        }
        else
        {
            if (!op_pop_and_insert(&state)) goto exit;
        }
    }

    // By now, the node vector can not be empty!
    // Empty string or string consisting of spaces will fail because of await_infix=false and '()' will fail because of unexpected closing parenthesis
    
    // 5. Build result and return value
    if (out_res != NULL)
    {
        *out_res = *(Node**)vec_pop(&state.vec_nodes);
    }
    
    exit:
    // If parsing wasn't successful or result is discarded, free partial results
    if (state.result.type != PERR_SUCCESS || out_res == NULL)
    {
        if (state.result.error_token == SIZE_MAX)
        {
            state.result.error_token = state.curr_tok;
        }

        while (true)
        {
            Node **node = vec_pop(&state.vec_nodes);
            if (node == NULL) break;
            free_tree(*node);
        }
    }
    vec_destroy(&state.vec_nodes);
    vec_destroy(&state.vec_ops);
    return state.result;
}

/* Parsing algorithm ends here. The following functions can be used to invoke parsing conveniently. */

/*
Summary: Parses string, tokenized with default tokenizer, to abstract syntax tree
Returns: True if success, False otherwise
*/
bool parse_input(const ParsingContext *ctx, const char *input, ParsingResult *out_res)
{
    out_res->tokens = tokenize(input, &ctx->keywords_trie);
    out_res->error = parse_tokens(ctx, vec_count(&out_res->tokens), out_res->tokens.buffer, &out_res->tree);
    return out_res->error.type == PERR_SUCCESS;
}

Node *parse_easy(const ParsingContext *ctx, const char *input)
{
    ParsingResult res;
    if (!parse_input(ctx, input, &res))
    {
        free_result(&res, true);
        return NULL;
    }
    else
    {
        free_result(&res, false);
        return res.tree;
    }
}

void free_result(ParsingResult *result, bool also_free_tree)
{
    if (result->error.type != PERR_NULL)
    {
        free_tokens(&result->tokens);
        if (also_free_tree && result->error.type == PERR_SUCCESS)
        {
            free_tree(result->tree);
            result->tree = NULL;
        }
        result->error.type = PERR_NULL;
    }
}
