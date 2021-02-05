#include <stdio.h>
#include <time.h>

#include "../../engine/tree/tree_util.h"
#include "../../util/string_util.h"
#include "../../util/console_util.h"
#include "../../engine/parsing/parser.h"

#include "../simplification/simplification.h"
#include "arith_context.h"
#include "arith_evaluation.h"
#include "history.h"

ParsingContext __g_ctx;
LinkedList __g_composite_functions;

void init_arith_ctx()
{
    __g_ctx = get_arith_ctx();
    srand(time(NULL));
    __g_composite_functions = list_create(sizeof(RewriteRule));
}

/*
Summary: Sets arithmetic context stored in global variable
*/
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

/*
Summary: Only calls parser, does not perform any substitution
*/
bool arith_parse_raw(char *input, char *error_fmt, size_t prompt_len, Node **out_res)
{
    size_t error_pos;
    ParserError perr = parse_input(g_ctx, input, out_res, &error_pos);
    if (perr != PERR_SUCCESS)
    {
        if (is_interactive())
        {
            report_error("%*s^ ", error_pos + prompt_len, "");
        }
        report_error(error_fmt, perr_to_string(perr));
        return false;
    }
    return true;
}

/*
Summary:
    Parses input, does post-processing of input, gives feedback on command line
    Result is guaranteed not to contain any user-defined functions or pseudo-ops (deriv, $...)
Returns:
    True when input was successfully parsed, false when syntax error in input or semantical error while transforming
*/
bool arith_parse_and_postprocess(char *input, char *error_fmt, size_t prompt_len, Node **out_res)
{
    if (arith_parse_raw(input, error_fmt, prompt_len, out_res))
    {
        ListenerError l_err = arith_postprocess(out_res);
        if (l_err != LISTENERERR_SUCCESS)
        {
            report_error(error_fmt, listenererr_to_str(l_err));
            free_tree(*out_res);
            return false;
        }
        return true;
    }
    else
    {
        return false;
    }
}

ListenerError arith_postprocess(Node **tree)
{
    LinkedListIterator iterator = list_get_iterator(g_composite_functions);
    apply_ruleset_by_iterator(tree, (Iterator*)&iterator, NULL, SIZE_MAX);
    return simplify(tree);
}

const char *listenererr_to_str(ListenerError error)
{
    switch (error)
    {
        case LISTENERERR_SUCCESS:
            return "No error";
        case LISTENERERR_VARIABLE_ENCOUNTERED:
            return "Expression not constant";
        case LISTENERERR_HISTORY_NOT_SET:
            return "This part of the history is not set yet";
        case LISTENERERR_IMPOSSIBLE_DERIV:
            return "Derivation not possible";
        case LISTENERERR_MALFORMED_DERIV_A:
            return "You can only use expr' when there is not more than one variable in expr.";
        case LISTENERERR_MALFORMED_DERIV_B:
            return "Second operand of function 'deriv' must be variable.";
        case LISTENERERR_UNKNOWN_OP:
            return "No evaluation of operator possible";
        default:
            return "Unknown error";
    }
}
