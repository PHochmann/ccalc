#include <stdio.h>
#include <time.h>

#include "arith_context.h"
#include "simplification.h"
#include "history.h"
#include "../util/string_util.h"
#include "../util/console_util.h"
#include "../parsing/parser.h"

#define NUM_OPS       56
#define NUM_COMP_FUNC 10
#define MAX_OPS (NUM_OPS + NUM_COMP_FUNC)

ParsingContext __g_ctx;
Operator operators[MAX_OPS];

Vector composite_functions;

/*
Summary: Sets arithmetic context stored in global variable
*/
void init_core_ctx()
{
    __g_ctx = get_context(MAX_OPS, operators);
    if (!ctx_add_ops(g_ctx, NUM_OPS,
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
    composite_functions = get_empty_ruleset();
}

size_t get_num_composite_functions()
{
    return vec_count(&composite_functions);
}

RewriteRule *get_composite_function(size_t index)
{
    return (RewriteRule*)vec_get(&composite_functions, index);
}

bool can_add_composite_function()
{
    return get_num_composite_functions() < NUM_COMP_FUNC;
}

void add_composite_function(RewriteRule rule)
{
    add_to_ruleset(&composite_functions, rule);
}

void pop_composite_function()
{
    if (get_num_composite_functions() > 0)
    {
        free_rule(VEC_POP_ELEM(&composite_functions, RewriteRule));
        free(operators[NUM_OPS + get_num_composite_functions()].name);
        g_ctx->num_ops--;
    }
}

void clear_composite_functions()
{
    while (get_num_composite_functions() > 0)
    {
        pop_composite_function();
    }
}

/*
Summary:
    Parses input, does post-processing of input, gives feedback on command line
Returns:
    True when input was successfully parsed, false when syntax error in input or semantical error while transforming
*/
bool arith_parse_input(char *input, char *error_fmt, bool replace_comp_funcs, Node **out_res)
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
            apply_ruleset(out_res, &composite_functions);
        }

        if (!core_simplify(out_res) || !core_replace_history(out_res))
        {
            free_tree(*out_res);
            return false;
        }

        return true;
    }
}
