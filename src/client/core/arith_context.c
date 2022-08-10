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
        op_get_function("median", OP_DYNAMIC_ARITY),
        op_get_function("gcd", 2),
        op_get_function("lcm", 2),
        op_get_function("rand", 2),
        op_get_function("fib", 1),
        op_get_function("gamma", 1),
        op_get_function("var", OP_DYNAMIC_ARITY),
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
    RewriteRule *rule = (RewriteRule*)listnode_get_data(node);
    char *temp = get_op(rule->pattern.pattern)->name;
    // Remove function operator from context
    ctx_delete_op(g_ctx, get_op(rule->pattern.pattern)->name, OP_PLACE_FUNCTION);
    // Free its name since it is malloced by the tokenizer in definition-command
    free(temp);
    // Free elimination rule
    free_rule(rule);
    // Remove from linked list
    list_delete_node(g_composite_functions, node);
}

bool remove_composite_function(const Operator *function)
{
    // Search for node in linked list to remove
    ListNode *curr = __g_composite_functions.first;
    while (curr != NULL)
    {
        RewriteRule *rule = (RewriteRule*)listnode_get_data(curr);
        if (get_op(rule->pattern.pattern) == function)
        {
            remove_node(curr);
            return true;
        }
        curr = listnode_get_next(curr);
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

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ WRAPPER FUNCTIONS FOR PARSER

static void get_pos_and_length_from_tokenstream(const Vector *tokens, size_t error_index, size_t *out_pos, size_t *out_length)
{
    *out_pos = 0;
    for (size_t i = 0; i < error_index; i++)
    {
        *out_pos += strlen(*(const char**)vec_get(tokens, i));
    }

    *out_length = 1;
    if (error_index < vec_count(tokens))
    {
        *out_length = strlen(*(const char**)vec_get(tokens, error_index));
    }
}

static void report_listener_error(const Vector *tokens, size_t token_index, int error_code, size_t prompt_len)
{
    size_t pos;
    size_t length;
    get_pos_and_length_from_tokenstream(tokens, token_index, &pos, &length);
    length += prompt_len;

    switch (error_code)
    {
        case LISTENERERR_SUCCESS:
            report_error_at(pos, length, "No error");
            break;
        case LISTENERERR_VARIABLE_ENCOUNTERED:
            report_error_at(pos, length, "Expression not constant");
            break;
        case LISTENERERR_HISTORY_NOT_SET:
            report_error_at(pos, length, "This part of the history is not set yet");
            break;
        case LISTENERERR_IMPOSSIBLE_DERIV:
            report_error_at(pos, length, "Differentiation of this expression not supported (yet)");
            break;
        case LISTENERERR_MALFORMED_DERIV_A:
            report_error_at(pos, length, "More than one variable in expr'");
            break;
        case LISTENERERR_MALFORMED_DERIV_B:
            report_error_at(pos, length, "Second operand of function 'deriv' must be variable");
            break;
        case LISTENERERR_UNKNOWN_OP:
            report_error_at(pos, length, "No evaluation of operator possible");
            break;
        case LISTENERERR_DIVISION_BY_ZERO:
            report_error_at(pos, length, "Division by zero");
            break;
        case LISTENERERR_COMPLEX_SOLUTION:
            report_error_at(pos, length, "Complex solution");
            break;
        case LISTENERERR_EMPTY_PARAMS:
            report_error_at(pos, length, "At least one operand needed");
            break;
        default:
            report_error_at(pos, length, "Unknown error");
            break;
    }
}

/*
Summary: Prints error message with position (if interactive) under token stream in console
*/
static void report_parser_error(const Vector *tokens, ParserError error, size_t prompt_len)
{
    size_t pos;
    size_t length;
    get_pos_and_length_from_tokenstream(tokens, error.error_token, &pos, &length);
    length += prompt_len;

    switch (error.type)
    {
        case PERR_SUCCESS:
            report_error_at(pos, length, "Success");
            break;
        case PERR_EXPECTED_INFIX:
            report_error_at(pos, length, "Expected infix or postfix operator");
            break;
        case PERR_UNEXPECTED_INFIX:
            report_error_at(pos, length, "Unexpected infix or postfix operator");
            break;
        case PERR_EXCESS_OPENING_PAREN:
            report_error_at(pos, length, "Missing closing parenthesis");
            break;
        case PERR_UNEXPECTED_CLOSING_PAREN:
            report_error_at(pos, length, "Unexpected closing parenthesis");
            break;
        case PERR_UNEXPECTED_DELIMITER:
            report_error_at(pos, length, "Unexpected delimiter");
            break;
        case PERR_FUNCTION_WRONG_ARITY:
            report_error_at(pos, length, "Wrong number of operands, got %d but expected %d",
                error.additional_data[0],
                error.additional_data[1]);
            break;
        case PERR_UNEXPECTED_END_OF_EXPR:
            report_error_at(pos, length, "Unexpected end of expression");
            break;
        case PERR_EXPECTED_PARAM_LIST:
            report_error_at(pos, length, "Expected an opening parenthesis");
            break;
        case PERR_UNEXPECTED_CHARACTER:
            report_error_at(pos, length, "Unexpected character");
            break;
        default:
            report_error_at(pos, length, "Unknown Error");
            break;
    }
}

bool arith_parse(char *input, size_t prompt_len, Node **out_res)
{
    ParsingResult res;
    if (arith_parse_raw(input, prompt_len, &res))
    {
        *out_res = arith_simplify(&res, prompt_len);
        if (*out_res == NULL) return false;
        return true;
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
        report_parser_error(&out_res->tokens, out_res->error, prompt_len);
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
    Will call free_result on p_result!!! Don't use it afterwards.
*/
Node *arith_simplify(ParsingResult *p_result, size_t prompt_len)
{
    LinkedListIterator iterator = list_get_iterator(g_composite_functions);
    apply_ruleset_by_iterator(&p_result->tree, (Iterator*)&iterator, NULL, SIZE_MAX);
    const Node *errnode = NULL;
    ListenerError l_err = simplify(&p_result->tree, &errnode);
    if (l_err != LISTENERERR_SUCCESS)
    {
        report_listener_error(&p_result->tokens, get_token_index(errnode), l_err, prompt_len);
        free_result(p_result, true);
        return NULL;
    }
    else
    {
        Node *res = p_result->tree;
        free_result(p_result, false);
        return res;
    }
}
