#include <stdio.h>
#include <time.h>

#include "arith_context.h"
#include "simplification.h"
#include "history.h"
#include "../tree/tree_util.h"
#include "../util/string_util.h"
#include "../util/console_util.h"
#include "../parsing/parser.h"

ParsingContext __g_ctx;
LinkedList g_composite_functions;

/*
Summary: Sets arithmetic context stored in global variable
*/
void init_arith_ctx()
{
    __g_ctx = ctx_create();
    if (!ctx_add_ops(g_ctx, NUM_PREDEFINED_OPS,
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
        op_get_constant("csound")))
    {
        report_error("Software defect: Inconsistent operator set.\n");
    }
    // Set multiplication as glue-op
    ctx_set_glue_op(g_ctx, ctx_lookup_op(g_ctx, "*", OP_PLACE_INFIX));
    srand(time(NULL));
    g_composite_functions = list_create(sizeof(RewriteRule));
}

void unload_arith_ctx()
{
    clear_composite_functions();
    list_destroy(&g_composite_functions);
    ctx_destroy(g_ctx);
}

void add_composite_function(RewriteRule rule)
{
    list_append(&g_composite_functions, (void*)(&rule));
}

// Removes node from g_composite_functions
void remove_node(ListNode *node)
{
    RewriteRule *rule = (RewriteRule*)node->data;
    char *temp = get_op(rule->before)->name;
    // Remove function operator from context
    ctx_delete_op(g_ctx, get_op(rule->before)->name, OP_PLACE_FUNCTION);
    // Free its name since it is malloced by the tokenizer in definition-command
    free(temp);
    // Free elimination rule
    free_rule(*(RewriteRule*)node->data);
    // Remove from linked list
    list_delete_node(&g_composite_functions, node);
}

bool remove_composite_function(Operator *function)
{
    // First check if another composite function depends on the operator
    ListNode *curr = g_composite_functions.first;
    while (curr != NULL)
    {
        RewriteRule *rule = (RewriteRule*)curr->data;
        if (find_op(&rule->after, function) != NULL)
        {
            report_error("This function can not be removed as it is used in another function's definition.\n");
            return false;
        }
        curr = curr->next;
    }

    // Search for node in linked list to remove
    curr = g_composite_functions.first;
    while (curr != NULL)
    {
        RewriteRule *rule = (RewriteRule*)curr->data;
        if (get_op(rule->before) == function)
        {
            remove_node(curr);
            return true;
        }
        curr = curr->next;
    }

    report_error("Built-in functions can not be removed.\n");
    return false;
}

void clear_composite_functions()
{
    while (list_count(&g_composite_functions) != 0)
    {
        remove_node(g_composite_functions.first);
    }
}

RewriteRule *get_composite_function(Operator *op)
{
    ListNode *curr = g_composite_functions.first;
    while (curr != NULL)
    {
        RewriteRule *rule = (RewriteRule*)curr->data;
        if (get_op(rule->before) == op)
        {
            return rule;
        }
    }
    return NULL;
}

bool arith_parse_input_raw(char *input, char *error_fmt, Node **out_res)
{
    return arith_parse_input(input, error_fmt, false, false, out_res);
}

/*
Summary:
    Parses input, does post-processing of input, gives feedback on command line
Returns:
    True when input was successfully parsed, false when syntax error in input or semantical error while transforming
*/
bool arith_parse_input(char *input, char *error_fmt, bool replace_comp_funcs, bool simplify, Node **out_res)
{
    ParserError perr = parse_input(g_ctx, input, out_res);
    if (perr != PERR_SUCCESS)
    {
        report_error(error_fmt, perr_to_string(perr));
        return false;
    }
    else
    {
        if (replace_comp_funcs)
        {
            apply_rule_list(out_res, &g_composite_functions);
        }

        if (!core_replace_history(out_res) || !core_simplify(out_res, simplify))
        {
            free_tree(*out_res);
            return false;
        }

        return true;
    }
}
