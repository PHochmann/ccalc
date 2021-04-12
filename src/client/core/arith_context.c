#include <stdio.h>
#include <time.h>
#include <string.h>

#include "../../engine/tree/tree_util.h"
#include "../../util/string_util.h"
#include "../../util/console_util.h"

#include "../simplification/simplification.h"
#include "arith_context.h"
#include "arith_evaluation.h"
#include "history.h"

ParsingContext __g_ctx;
LinkedList __g_composite_functions;

/*
Summary: Sets arithmetic context stored in global variable
*/
void init_arith_ctx()
{
    srand(time(NULL));
    __g_ctx = get_arith_ctx();
    __g_composite_functions = list_create(sizeof(RewriteRule));
}

ParsingContext get_arith_ctx()
{
    ParsingContext res = ctx_create();
    if (!ctx_add_ops(&res, NUM_ARITH_OPS,
        op_get_prefix("$", 0),
        op_get_prefix("@", 8),
        op_get_postfix("'", 7),
        op_get_function("deriv", 2),
        op_get_infix("+", 2, OP_ASSOC_LEFT),
        op_get_infix("-", 2, OP_ASSOC_LEFT),
        op_get_infix("*", 4, OP_ASSOC_LEFT),
        op_get_infix("/", 3, OP_ASSOC_LEFT),
        op_get_infix("^", 5, OP_ASSOC_RIGHT),
        op_get_infix("C", 1, OP_ASSOC_LEFT),
        op_get_infix("mod", 1, OP_ASSOC_LEFT),
        op_get_prefix("+", 7),
        op_get_prefix("-", 7),
        op_get_postfix("!", 6),
        op_get_postfix("%", 6),
        op_get_function("exp", 1),
        op_get_function("root", 2),
        op_get_function("sqrt", 1),
        op_get_function("log", 2),
        op_get_function("ln", 1),
        op_get_function("ld", 1),
        op_get_function("lg", 1),
        op_get_function("sin", 1),
        op_get_function("cos", 1),
        op_get_function("tan", 1),
        op_get_function("asin", 1),
        op_get_function("acos", 1),
        op_get_function("atan", 1),
        op_get_function("sinh", 1),
        op_get_function("cosh", 1),
        op_get_function("tanh", 1),
        op_get_function("asinh", 1),
        op_get_function("acosh", 1),
        op_get_function("atanh", 1),
        op_get_function("max", OP_DYNAMIC_ARITY),
        op_get_function("min", OP_DYNAMIC_ARITY),
        op_get_function("abs", 1),
        op_get_function("ceil", 1),
        op_get_function("floor", 1),
        op_get_function("round", 1),
        op_get_function("trunc", 1),
        op_get_function("frac", 1),
        op_get_function("sgn", 1),
        op_get_function("sum", OP_DYNAMIC_ARITY),
        op_get_function("prod", OP_DYNAMIC_ARITY),
        op_get_function("avg", OP_DYNAMIC_ARITY),
        op_get_function("gcd", 2),
        op_get_function("lcm", 2),
        op_get_function("rand", 2),
        op_get_function("fib", 1),
        op_get_function("gamma", 1),
        op_get_constant("pi"),
        op_get_constant("e"),
        op_get_constant("phi"),
        op_get_constant("clight"),
        op_get_constant("csound"),
        op_get_constant("ans")))
    {
        software_defect("[Arith] Inconsistent operator set.\n");
    }
    // Set multiplication as glue-op
    ctx_set_glue_op(&res, ctx_lookup_op(&res, "*", OP_PLACE_INFIX));
    return res;
}

void unload_arith_ctx()
{
    clear_composite_functions();
    list_destroy(g_composite_functions);
    ctx_destroy(g_ctx);
}

void add_composite_function(RewriteRule rule)
{
    list_append(g_composite_functions, (void*)&rule);
}

// Removes node from g_composite_functions
static void remove_node(ListNode *node)
{
    RewriteRule *rule = (RewriteRule*)node->data;
    char *temp = get_op(rule->pattern.pattern)->name;
    // Remove function operator from context
    ctx_delete_op(g_ctx, get_op(rule->pattern.pattern)->name, OP_PLACE_FUNCTION);
    // Free its name since it is malloced by the tokenizer in definition-command
    free(temp);
    // Free elimination rule
    free_rule((RewriteRule*)node->data);
    // Remove from linked list
    list_delete_node(g_composite_functions, node);
}

bool remove_composite_function(const Operator *function)
{
    // Search for node in linked list to remove
    ListNode *curr = __g_composite_functions.first;
    while (curr != NULL)
    {
        RewriteRule *rule = (RewriteRule*)curr->data;
        if (get_op(rule->pattern.pattern) == function)
        {
            remove_node(curr);
            return true;
        }
        curr = curr->next;
    }
    // Operator is not in list of composite functions, it must be built in
    report_error("Built-in functions can not be removed\n");
    return false;
}

void clear_composite_functions()
{
    while (list_count(g_composite_functions) != 0)
    {
        remove_node(__g_composite_functions.first);
    }
}

RewriteRule *get_composite_function(Operator *op)
{
    ListNode *curr = __g_composite_functions.first;
    while (curr != NULL)
    {
        RewriteRule *rule = (RewriteRule*)curr->data;
        if (get_op(rule->pattern.pattern) == op)
        {
            return rule;
        }
    }
    return NULL;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ WRAPPER FUNCTIONS FOR PARSER

/*
Returns: String representation of ParserError
*/
static const char *perr_to_string(ParserError perr)
{
    switch (perr)
    {
        case PERR_SUCCESS:
            return "Success";
        case PERR_EXPECTED_INFIX:
            return "Expected infix or postfix operator";
        case PERR_UNEXPECTED_INFIX:
            return "Unexpected infix or postfix operator";
        case PERR_EXCESS_OPENING_PARENTHESIS:
            return "Missing closing parenthesis";
        case PERR_UNEXPECTED_CLOSING_PARENTHESIS:
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
        case PERR_UNEXPECTED_END_OF_EXPR:
            return "Unexpected end of expression";
        case PERR_EXPECTED_PARAM_LIST:
            return "Expected an opening parenthesis";
        default:
            return "Unknown Error";
    }
}

static const char *listenererr_to_str(int code)
{
    switch (code)
    {
        case LISTENERERR_SUCCESS:
            return "No error";
        case LISTENERERR_VARIABLE_ENCOUNTERED:
            return "Expression not constant";
        case LISTENERERR_HISTORY_NOT_SET:
            return "This part of the history is not set yet";
        case LISTENERERR_IMPOSSIBLE_DERIV:
            return "Expression not continuously differentiable";
        case LISTENERERR_MALFORMED_DERIV_A:
            return "More than one variable in expr'";
        case LISTENERERR_MALFORMED_DERIV_B:
            return "Second operand of function 'deriv' must be variable";
        case LISTENERERR_UNKNOWN_OP:
            return "No evaluation of operator possible";
        case LISTENERERR_DIVISION_BY_ZERO:
            return "Division by zero";
        case LISTENERERR_COMPLEX_SOLUTION:
            return "Complex solution";
        default:
            return "Unknown error";
    }
}

/*
Summary: Prints error message with position (if interactive) under token stream in console
*/
static void show_error_at_token(const Vector *tokens, size_t error_token, const char *message, size_t prompt_len)
{
    int error_pos = prompt_len;
    for (size_t i = 0; i < error_token; i++)
    {
        error_pos += strlen(*(const char**)vec_get(tokens, i));
    }
    int error_length = 1;
    if (error_token < vec_count(tokens)) error_length = strlen(*(const char**)vec_get(tokens, error_token));
    report_error_at(error_pos, error_length, "Error: %s", message);
}

bool arith_parse(char *input, size_t prompt_len, Node **out_res)
{
    ParsingResult res;
    if (arith_parse_raw(input, prompt_len, &res))
    {
        if (arith_postprocess(&res, prompt_len))
        {
            free_result(&res, false);
            *out_res = res.tree;
            return true;
        }
    }
    return false;
}

/*
Summary: Only calls parser, does not perform any substitution
*/
bool arith_parse_raw(char *input, size_t prompt_len, ParsingResult *out_res)
{
    if (!parse_input(g_ctx, input, out_res))
    {
        show_error_at_token(&out_res->tokens, out_res->error_token, perr_to_string(out_res->error), prompt_len);
        free_result(out_res, false);
        return false;
    }
    else
    {
        return true;
    }
}

/*
Summary: Replaces user-defined functions and simplifies
*/
bool arith_postprocess(ParsingResult *p_result, size_t prompt_len)
{
    LinkedListIterator iterator = list_get_iterator(g_composite_functions);
    apply_ruleset_by_iterator(&p_result->tree, (Iterator*)&iterator, NULL, SIZE_MAX);
    const Node *errnode = NULL;
    ListenerError l_err = simplify(&p_result->tree, &errnode);
    if (l_err != LISTENERERR_SUCCESS)
    {
        show_error_at_token(&p_result->tokens, get_token_index(errnode), listenererr_to_str(l_err), prompt_len);
        free_result(p_result, true);
        return false;
    }
    else
    {
        return true;
    }
}
