#include <stdbool.h>
#include <string.h>

#include "tokenizer.h"
#include "parser.h"
#include "../util/string_util.h"
#include "../util/console_util.h"

#define VECTOR_STARTSIZE 10

// Do not use this macro in auxiliary functions!
#define ERROR(type) {\
    state.result = type;\
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
    if (popped == NULL)
    {
        state->result = PERR_MISSING_OPERAND;
        return false;
    }
    *out = *popped;
    return true;
}

bool op_pop_and_insert(struct ParserState *state)
{
    struct OpData *op_data = (struct OpData*)vec_pop(&state->vec_ops);
    if (op_data == NULL)
    {
        state->result = PERR_MISSING_OPERATOR;
        return false;
    }

    const Operator *op = op_data->op;

    if (op != NULL) // Construct operator-node and append children
    {
        if (op->arity != OP_DYNAMIC_ARITY && op->arity != op_data->arity)
        {
            state->result = PERR_FUNCTION_WRONG_ARITY;
            state->curr_tok = op_data->token;
            return false;
        }

        // We try to allocate a new node and pop its children from node stack
        Node *op_node = malloc_operator_node(op, op_data->arity);
        
        for (size_t i = 0; i < get_num_children(op_node); i++)
        {
            // Pop nodes from stack and append them in subtree
            if (!node_pop(state, get_child_addr(op_node, get_num_children(op_node) - i - 1)))
            {
                // Free already appended children and new node on error
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
ParserError parse_tokens(const ParsingContext *ctx, size_t num_tokens, const char **tokens, Node **out_res, size_t *error_token)
{
    // 1. Early outs
    if (ctx == NULL || tokens == NULL) return PERR_ARGS_MALFORMED;

    // 2. Initialize state
    struct ParserState state = {
        .ctx       = ctx,
        .result    = PERR_SUCCESS,
        .vec_nodes = vec_create(sizeof(Node*), VECTOR_STARTSIZE),
        .vec_ops   = vec_create(sizeof(struct OpData), VECTOR_STARTSIZE)
    };

    // 3. Process each token
    bool await_infix = false; // Or postfix, or delimiter, or closing parenthesis
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
            if (!push_opening_parenthesis(&state)) goto exit;
            continue;
        }

        // III. Is token closing parenthesis or argument delimiter?
        if (is_closing_parenthesis(token))
        {
            await_infix = true;

            // Pop ops until opening parenthesis on op-stack
            while (op_peek(&state) != NULL && op_peek(&state)->op != NULL)
            {
                if (!op_pop_and_insert(&state))
                {
                    ERROR(PERR_EXCESS_CLOSING_PARENTHESIS);
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
                ERROR(PERR_EXCESS_CLOSING_PARENTHESIS);
            }
            
            // Increment operand count one last time if it was not the empty parameter list.
            if (op_peek(&state) != NULL
                && op_peek(&state)->op != NULL
                && op_peek(&state)->op->placement == OP_PLACE_FUNCTION
                && i > 0
                && !is_opening_parenthesis(tokens[i - 1]))
            {
                op_peek(&state)->arity++;
            }
            
            continue;
        }
        
        if (is_delimiter(token))
        {
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

                // Directly pop constant functions or functions with no parameter list
                if (op->arity == 0 || (i < num_tokens - 1 && !is_opening_parenthesis(tokens[i + 1])))
                {
                    // Skip parsing of empty parameter list
                    if ((int)i < (int)num_tokens - 2 && is_opening_parenthesis(tokens[i + 1])
                        && is_closing_parenthesis(tokens[i + 2]))
                    {
                        i += 2;
                    }

                    if (!op_pop_and_insert(&state)) goto exit;
                    await_infix = true;
                    continue;
                }

                await_infix = false;
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
                // Postfix operators are never on the op_stack, because their operands are directly available
                op_pop_and_insert(&state);
                await_infix = true;
                continue;
            }
            
            // We can fail here: no more tokens processable (no glue-op)
            ERROR(PERR_UNEXPECTED_SUBEXPRESSION);
        }
        
        // V. Token must be variable or constant (leaf)
        Node *node;

        // Is token constant?
        double const_val;
        if (try_parse_constant(token, &const_val))
        {
            node = malloc_constant_node(const_val);
        }
        else // Token must be variable
        {
            node = malloc_variable_node(token, 0);
        }

        await_infix = true;
        node_push(&state, node);
    }

    state.curr_tok = num_tokens;
    
    // 4. Pop all remaining operators
    while (op_peek(&state) != NULL)
    {
        if (op_peek(&state)->op == NULL)
        {
            ERROR(PERR_EXCESS_OPENING_PARENTHESIS);
        }
        else
        {
            if (!op_pop_and_insert(&state)) goto exit;
        }
    }
    
    // 5. Build result and return value
    switch (vec_count(&state.vec_nodes))
    {
        // We haven't constructed a single node
        case 0:
            state.result = PERR_EMPTY;
            break;
        // We successfully constructed a single AST
        case 1:
            if (out_res != NULL) *out_res = *(Node**)vec_pop(&state.vec_nodes);
            break;
        // We have multiple ASTs (need glue-op)
        default:
            state.result = PERR_MISSING_OPERATOR;
    }
    
    exit:
    // If parsing wasn't successful or result is discarded, free partial results
    if (state.result != PERR_SUCCESS || out_res == NULL)
    {
        if (error_token != NULL)
        {
            *error_token = state.curr_tok;
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
Returns: Result code to indicate whether string was parsed successfully or which error occurred
*/
ParserError parse_input(const ParsingContext *ctx, const char *input, Node **out_res, size_t *error_pos)
{
    Vector tokens;
    tokenize(input, &ctx->keywords_trie, &tokens);
    size_t error_token = 0;
    ParserError result = parse_tokens(ctx, vec_count(&tokens), tokens.buffer, out_res, &error_token);
    if (result != PERR_SUCCESS && error_pos != NULL)
    {
        *error_pos = 0;
        for (size_t i = 0; i < error_token; i++)
        {
            *error_pos += strlen(*(const char**)vec_get(&tokens, i));
        }
    }
    free_tokens(&tokens);
    return result;
}

/*
Summary: Calls parse_input, omits ParserError
Returns: Operator tree or NULL when error occurred
*/
Node *parse_conveniently(const ParsingContext *ctx, const char *input)
{
    Node *result = NULL;
    ParserError error = parse_input(ctx, input, &result, NULL);
    if (result == NULL)
    {
        report_error("Syntax error: %s\n", perr_to_string(error));
    }
    return result;
}

/*
Returns: String representation of ParserError
*/
const char *perr_to_string(ParserError perr)
{
    switch (perr)
    {
        case PERR_SUCCESS:
            return "Success";
        case PERR_UNEXPECTED_SUBEXPRESSION:
            return "Unexpected subexpression";
        case PERR_EXCESS_OPENING_PARENTHESIS:
            return "Missing closing parenthesis";
        case PERR_EXCESS_CLOSING_PARENTHESIS:
            return "Unexpected closing parenthesis";
        case PERR_UNEXPECTED_DELIMITER:
            return "Unexpected delimiter";
        case PERR_MISSING_OPERATOR:
            return "Unexpected operand";
        case PERR_MISSING_OPERAND:
            return "Missing operand";
        case PERR_FUNCTION_WRONG_ARITY:
            return "Wrong number of operands of function";
        case PERR_CHILDREN_EXCEEDED:
            return "Exceeded maximum number of operands of function";
        case PERR_EMPTY:
            return "Empty Expression";
        default:
            return "Unknown Error";
    }
}
